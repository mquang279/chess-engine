#include "Evaluation.hpp"

int Evaluation::evaluate(const chess::Board &board) const
{
    // Determine perspective of evaluation (positive is good for side to move)
    int perspective = board.sideToMove() == chess::Color::WHITE ? 1 : -1;

    // Check for checkmate/stalemate
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    if (moves.empty())
    {
        if (board.inCheck())
        {
            // Checkmate is worst possible eval, adjusted for depth to prefer mates in fewer moves
            return -30000 * perspective;
        }
        else
        {
            // Stalemate is a draw (0)
            return 0;
        }
    }

    bool endgame = isEndgame(board);

    // Calculate various evaluation components
    int materialScore = evaluateMaterial(board);
    int positionalScore = evaluatePosition(board, endgame);
    int pawnStructureScore = evaluatePawnStructure(board);

    // Calculate final score
    int score = materialScore + positionalScore + pawnStructureScore;

    // Return score from side to move's perspective
    return score * perspective;
}

int Evaluation::evaluateMaterial(const chess::Board &board) const
{
    int whiteScore = 0;
    int blackScore = 0;

    // Calculate material for each piece type
    for (int pt = 0; pt < 6; pt++)
    {
        chess::PieceType pieceType = static_cast<chess::PieceType>(pt);
        whiteScore += chess::builtin::popcount(board.pieces(pieceType, chess::Color::WHITE)) * PIECE_VALUES[pt];
        blackScore += chess::builtin::popcount(board.pieces(pieceType, chess::Color::BLACK)) * PIECE_VALUES[pt];
    }

    return whiteScore - blackScore;
}

bool Evaluation::isEndgame(const chess::Board &board) const
{
    // Consider it an endgame if:
    // 1. Both sides have no queens, or
    // 2. The side with a queen has <= 1 minor piece and the other side has <= 2 minors

    bool whiteHasQueen = board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE) != 0;
    bool blackHasQueen = board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK) != 0;

    if (!whiteHasQueen && !blackHasQueen)
    {
        return true;
    }

    int whiteMinors =
        chess::builtin::popcount(board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE));

    int blackMinors =
        chess::builtin::popcount(board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK));

    if ((whiteHasQueen && whiteMinors <= 1 && !blackHasQueen && blackMinors <= 2) ||
        (blackHasQueen && blackMinors <= 1 && !whiteHasQueen && whiteMinors <= 2))
    {
        return true;
    }

    // Check total material - if less than threshold, consider it endgame
    int totalMaterial =
        chess::builtin::popcount(board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::ROOK, chess::Color::WHITE)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::ROOK, chess::Color::BLACK)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE)) +
        chess::builtin::popcount(board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK));

    return totalMaterial <= 6;
}

int Evaluation::evaluatePosition(const chess::Board &board, bool isEndgame) const
{
    int whiteScore = 0;
    int blackScore = 0;

    // Evaluate positional value of each piece
    chess::Bitboard whitePawns = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE);
    chess::Bitboard blackPawns = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);
    chess::Bitboard whiteKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE);
    chess::Bitboard blackKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK);
    chess::Bitboard whiteBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE);
    chess::Bitboard blackBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK);
    chess::Bitboard whiteRooks = board.pieces(chess::PieceType::ROOK, chess::Color::WHITE);
    chess::Bitboard blackRooks = board.pieces(chess::PieceType::ROOK, chess::Color::BLACK);
    chess::Bitboard whiteQueens = board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE);
    chess::Bitboard blackQueens = board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK);

    // Process each piece type
    while (whitePawns)
    {
        int sq = chess::builtin::poplsb(whitePawns);
        whiteScore += PAWN_PST[sq];
    }

    while (blackPawns)
    {
        int sq = chess::builtin::poplsb(blackPawns);
        blackScore += PAWN_PST[mirrorSquare(sq)];
    }

    while (whiteKnights)
    {
        int sq = chess::builtin::poplsb(whiteKnights);
        whiteScore += KNIGHT_PST[sq];
    }

    while (blackKnights)
    {
        int sq = chess::builtin::poplsb(blackKnights);
        blackScore += KNIGHT_PST[mirrorSquare(sq)];
    }

    while (whiteBishops)
    {
        int sq = chess::builtin::poplsb(whiteBishops);
        whiteScore += BISHOP_PST[sq];
    }

    while (blackBishops)
    {
        int sq = chess::builtin::poplsb(blackBishops);
        blackScore += BISHOP_PST[mirrorSquare(sq)];
    }

    while (whiteRooks)
    {
        int sq = chess::builtin::poplsb(whiteRooks);
        whiteScore += ROOK_PST[sq];
    }

    while (blackRooks)
    {
        int sq = chess::builtin::poplsb(blackRooks);
        blackScore += ROOK_PST[mirrorSquare(sq)];
    }

    while (whiteQueens)
    {
        int sq = chess::builtin::poplsb(whiteQueens);
        whiteScore += QUEEN_PST[sq];
    }

    while (blackQueens)
    {
        int sq = chess::builtin::poplsb(blackQueens);
        blackScore += QUEEN_PST[mirrorSquare(sq)];
    }

    // King position evaluation depends on game phase
    int whiteKingSq = chess::builtin::lsb(board.pieces(chess::PieceType::KING, chess::Color::WHITE));
    int blackKingSq = chess::builtin::lsb(board.pieces(chess::PieceType::KING, chess::Color::BLACK));

    if (isEndgame)
    {
        whiteScore += KING_END_PST[whiteKingSq];
        blackScore += KING_END_PST[mirrorSquare(blackKingSq)];
    }
    else
    {
        whiteScore += KING_MIDDLE_PST[whiteKingSq];
        blackScore += KING_MIDDLE_PST[mirrorSquare(blackKingSq)];
    }

    // Bishop pair bonus
    if (chess::builtin::popcount(whiteBishops) >= 2)
    {
        whiteScore += 30;
    }

    if (chess::builtin::popcount(blackBishops) >= 2)
    {
        blackScore += 30;
    }

    return whiteScore - blackScore;
}

int Evaluation::evaluatePawnStructure(const chess::Board &board) const
{
    int whiteScore = 0;
    int blackScore = 0;

    chess::Bitboard whitePawns = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE);
    chess::Bitboard blackPawns = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);

    // Evaluate pawn islands (groups of pawns separated by empty files)
    // Fewer islands is usually better
    chess::Bitboard whitePawnFiles = 0;
    chess::Bitboard blackPawnFiles = 0;

    chess::Bitboard whitePawnsCopy = whitePawns;
    chess::Bitboard blackPawnsCopy = blackPawns;

    while (whitePawnsCopy)
    {
        int sq = chess::builtin::poplsb(whitePawnsCopy);
        whitePawnFiles |= 1ULL << (sq & 7); // Get file (0-7)
    }

    while (blackPawnsCopy)
    {
        int sq = chess::builtin::poplsb(blackPawnsCopy);
        blackPawnFiles |= 1ULL << (sq & 7); // Get file (0-7)
    }

    // Count islands (groups of consecutive set bits)
    int whiteIslands = 0;
    int blackIslands = 0;

    bool whitePrevFile = false;
    bool blackPrevFile = false;

    for (int file = 0; file < 8; file++)
    {
        bool whiteHasPawn = (whitePawnFiles & (1ULL << file)) != 0;
        bool blackHasPawn = (blackPawnFiles & (1ULL << file)) != 0;

        if (whiteHasPawn && !whitePrevFile)
        {
            whiteIslands++;
        }

        if (blackHasPawn && !blackPrevFile)
        {
            blackIslands++;
        }

        whitePrevFile = whiteHasPawn;
        blackPrevFile = blackHasPawn;
    }

    // Penalty for more than 1 island
    whiteScore -= (whiteIslands - 1) * 10;
    blackScore -= (blackIslands - 1) * 10;

    // Bonus for passed pawns
    whitePawnsCopy = whitePawns;
    while (whitePawnsCopy)
    {
        int sq = chess::builtin::poplsb(whitePawnsCopy);
        int file = sq & 7;
        int rank = sq >> 3;

        // Check if pawn is passed (no enemy pawns ahead on same or adjacent files)
        bool passed = true;
        chess::Bitboard ahead = 0;

        for (int r = rank + 1; r < 8; r++)
        {
            if (file > 0)
                ahead |= 1ULL << (r * 8 + file - 1);
            ahead |= 1ULL << (r * 8 + file);
            if (file < 7)
                ahead |= 1ULL << (r * 8 + file + 1);
        }

        if (ahead & blackPawns)
        {
            passed = false;
        }

        if (passed)
        {
            whiteScore += 20 + 5 * rank; // More bonus for pawns closer to promotion
        }
    }

    blackPawnsCopy = blackPawns;
    while (blackPawnsCopy)
    {
        int sq = chess::builtin::poplsb(blackPawnsCopy);
        int file = sq & 7;
        int rank = sq >> 3;

        // Check if pawn is passed
        bool passed = true;
        chess::Bitboard ahead = 0;

        for (int r = rank - 1; r >= 0; r--)
        {
            if (file > 0)
                ahead |= 1ULL << (r * 8 + file - 1);
            ahead |= 1ULL << (r * 8 + file);
            if (file < 7)
                ahead |= 1ULL << (r * 8 + file + 1);
        }

        if (ahead & whitePawns)
        {
            passed = false;
        }

        if (passed)
        {
            blackScore += 20 + 5 * (7 - rank); // More bonus for pawns closer to promotion
        }
    }

    return whiteScore - blackScore;
}