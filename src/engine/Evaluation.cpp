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

    // Calculate game phase for tapered evaluation (smooth transition between middlegame and endgame)
    int gamePhase = calculateGamePhase(board);

    // Calculate various evaluation components
    int materialScore = evaluateMaterial(board);
    int positionalScore = evaluatePosition(board, gamePhase);
    int pawnStructureScore = evaluatePawnStructure(board);
    int mobilityScore = evaluateMobility(board);
    int kingSafetyScore = evaluateKingSafety(board, gamePhase);
    int threatScore = evaluateThreats(board);
    int pieceCoordinationScore = evaluatePieceCoordination(board);

    // Calculate final score as weighted sum of components
    int score = materialScore + positionalScore + pawnStructureScore +
                mobilityScore + kingSafetyScore + threatScore + pieceCoordinationScore;

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

int Evaluation::evaluatePosition(const chess::Board &board, int gamePhase) const
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
        
        // Bonus for rooks on open or semi-open files
        int file = sq & 7;
        if (isOpenFile(board, file)) {
            whiteScore += 25;
        } else if (isSemiOpenFile(board, file, chess::Color::WHITE)) {
            whiteScore += 15;
        }
        
        // Bonus for rooks on 7th rank
        if ((sq >> 3) == 6) {
            whiteScore += 20;
        }
    }

    while (blackRooks)
    {
        int sq = chess::builtin::poplsb(blackRooks);
        blackScore += ROOK_PST[mirrorSquare(sq)];
        
        // Bonus for rooks on open or semi-open files
        int file = sq & 7;
        if (isOpenFile(board, file)) {
            blackScore += 25;
        } else if (isSemiOpenFile(board, file, chess::Color::BLACK)) {
            blackScore += 15;
        }
        
        // Bonus for rooks on 7th rank (2nd rank for black)
        if ((sq >> 3) == 1) {
            blackScore += 20;
        }
    }

    while (whiteQueens)
    {
        int sq = chess::builtin::poplsb(whiteQueens);
        whiteScore += QUEEN_PST[sq];
        
        // Bonus for queens on open or semi-open files
        int file = sq & 7;
        if (isOpenFile(board, file)) {
            whiteScore += 10;
        } else if (isSemiOpenFile(board, file, chess::Color::WHITE)) {
            whiteScore += 5;
        }
        
        // Bonus for queen on 7th rank
        if ((sq >> 3) == 6) {
            whiteScore += 10;
        }
    }

    while (blackQueens)
    {
        int sq = chess::builtin::poplsb(blackQueens);
        blackScore += QUEEN_PST[mirrorSquare(sq)];
        
        // Bonus for queens on open or semi-open files
        int file = sq & 7;
        if (isOpenFile(board, file)) {
            blackScore += 10;
        } else if (isSemiOpenFile(board, file, chess::Color::BLACK)) {
            blackScore += 5;
        }
        
        // Bonus for queen on 7th rank (2nd rank for black)
        if ((sq >> 3) == 1) {
            blackScore += 10;
        }
    }

    // King position evaluation based on game phase (tapered evaluation)
    int whiteKingSq = chess::builtin::lsb(board.pieces(chess::PieceType::KING, chess::Color::WHITE));
    int blackKingSq = chess::builtin::lsb(board.pieces(chess::PieceType::KING, chess::Color::BLACK));

    // Interpolate between middlegame and endgame scores based on game phase
    int whiteKingMiddlegameScore = KING_MIDDLE_PST[whiteKingSq];
    int whiteKingEndgameScore = KING_END_PST[whiteKingSq];
    whiteScore += (whiteKingMiddlegameScore * gamePhase + whiteKingEndgameScore * (256 - gamePhase)) / 256;
    
    int blackKingMiddlegameScore = KING_MIDDLE_PST[mirrorSquare(blackKingSq)];
    int blackKingEndgameScore = KING_END_PST[mirrorSquare(blackKingSq)];
    blackScore += (blackKingMiddlegameScore * gamePhase + blackKingEndgameScore * (256 - gamePhase)) / 256;

    // Bishop pair bonus
    if (chess::builtin::popcount(whiteBishops) >= 2)
    {
        whiteScore += 30;
    }

    if (chess::builtin::popcount(blackBishops) >= 2)
    {
        blackScore += 30;
    }
    
    // Knight penalty in endgame with few pawns
    if (gamePhase < 64) { // Deep endgame
        int whitePawnCount = chess::builtin::popcount(board.pieces(chess::PieceType::PAWN, chess::Color::WHITE));
        int blackPawnCount = chess::builtin::popcount(board.pieces(chess::PieceType::PAWN, chess::Color::BLACK));
        
        if (whitePawnCount <= 2) {
            whiteScore -= 15 * chess::builtin::popcount(whiteKnights);
        }
        
        if (blackPawnCount <= 2) {
            blackScore -= 15 * chess::builtin::popcount(blackKnights);
        }
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

int Evaluation::calculateGamePhase(const chess::Board &board) const
{
    // Game phase is calculated as a function of the remaining material
    // Full game = 256, endgame = 0, with gradual blending between them
    
    constexpr int pawnPhase = 0;
    constexpr int knightPhase = 1;
    constexpr int bishopPhase = 1;
    constexpr int rookPhase = 2;
    constexpr int queenPhase = 4;
    constexpr int totalPhase = knightPhase * 4 + bishopPhase * 4 + rookPhase * 4 + queenPhase * 2;
    
    int phase = totalPhase;
    
    // Subtract material from the full phase value to determine current game phase
    phase -= knightPhase * chess::builtin::popcount(
        board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE) |
        board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK));
        
    phase -= bishopPhase * chess::builtin::popcount(
        board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE) |
        board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK));
        
    phase -= rookPhase * chess::builtin::popcount(
        board.pieces(chess::PieceType::ROOK, chess::Color::WHITE) |
        board.pieces(chess::PieceType::ROOK, chess::Color::BLACK));
        
    phase -= queenPhase * chess::builtin::popcount(
        board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE) |
        board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK));
    
    // Scale phase between 0 and 256
    phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
    
    // Ensure phase is within bounds
    if (phase < 0) phase = 0;
    if (phase > 256) phase = 256;
    
    return phase;
}

bool Evaluation::isOpenFile(const chess::Board &board, int file) const
{
    // A file is open if there are no pawns on it from either side
    chess::Bitboard fileMask = chess::attacks::MASK_FILE[file];
    
    chess::Bitboard allPawns = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE) |
                               board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);
    
    return !(allPawns & fileMask);
}

bool Evaluation::isSemiOpenFile(const chess::Board &board, int file, chess::Color color) const
{
    // A file is semi-open if there are no pawns of a specific color on it
    chess::Bitboard fileMask = chess::attacks::MASK_FILE[file];
    
    chess::Bitboard ourPawns = board.pieces(chess::PieceType::PAWN, color);
    
    return !(ourPawns & fileMask);
}

int Evaluation::evaluateMobility(const chess::Board &board) const
{
    int whiteScore = 0;
    int blackScore = 0;
    
    chess::Bitboard occupied = board.occ();
    chess::Bitboard whiteOcc = board.us(chess::Color::WHITE);
    chess::Bitboard blackOcc = board.us(chess::Color::BLACK);
    
    // Knights
    chess::Bitboard whiteKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE);
    while (whiteKnights) {
        int sq = chess::builtin::poplsb(whiteKnights);
        // Count legal moves for this knight, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::knight(static_cast<chess::Square>(sq)) & ~whiteOcc);
        whiteScore += moves * MOBILITY_BONUS[0];
    }
    
    chess::Bitboard blackKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK);
    while (blackKnights) {
        int sq = chess::builtin::poplsb(blackKnights);
        // Count legal moves for this knight, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::knight(static_cast<chess::Square>(sq)) & ~blackOcc);
        blackScore += moves * MOBILITY_BONUS[0];
    }
    
    // Bishops
    chess::Bitboard whiteBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE);
    while (whiteBishops) {
        int sq = chess::builtin::poplsb(whiteBishops);
        // Count legal moves for this bishop, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::bishop(static_cast<chess::Square>(sq), occupied) & ~whiteOcc);
        whiteScore += moves * MOBILITY_BONUS[1];
    }
    
    chess::Bitboard blackBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK);
    while (blackBishops) {
        int sq = chess::builtin::poplsb(blackBishops);
        // Count legal moves for this bishop, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::bishop(static_cast<chess::Square>(sq), occupied) & ~blackOcc);
        blackScore += moves * MOBILITY_BONUS[1];
    }
    
    // Rooks
    chess::Bitboard whiteRooks = board.pieces(chess::PieceType::ROOK, chess::Color::WHITE);
    while (whiteRooks) {
        int sq = chess::builtin::poplsb(whiteRooks);
        // Count legal moves for this rook, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::rook(static_cast<chess::Square>(sq), occupied) & ~whiteOcc);
        whiteScore += moves * MOBILITY_BONUS[2];
    }
    
    chess::Bitboard blackRooks = board.pieces(chess::PieceType::ROOK, chess::Color::BLACK);
    while (blackRooks) {
        int sq = chess::builtin::poplsb(blackRooks);
        // Count legal moves for this rook, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::rook(static_cast<chess::Square>(sq), occupied) & ~blackOcc);
        blackScore += moves * MOBILITY_BONUS[2];
    }
    
    // Queens
    chess::Bitboard whiteQueens = board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE);
    while (whiteQueens) {
        int sq = chess::builtin::poplsb(whiteQueens);
        // Count legal moves for this queen, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::queen(static_cast<chess::Square>(sq), occupied) & ~whiteOcc);
        whiteScore += moves * MOBILITY_BONUS[3];
    }
    
    chess::Bitboard blackQueens = board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK);
    while (blackQueens) {
        int sq = chess::builtin::poplsb(blackQueens);
        // Count legal moves for this queen, excluding captures of own pieces
        int moves = chess::builtin::popcount(chess::attacks::queen(static_cast<chess::Square>(sq), occupied) & ~blackOcc);
        blackScore += moves * MOBILITY_BONUS[3];
    }
    
    return whiteScore - blackScore;
}

int Evaluation::countPawnShield(const chess::Board &board, int kingSq, chess::Color color) const
{
    int kingFile = kingSq & 7;
    int kingRank = kingSq >> 3;
    int count = 0;
    
    // Define the area in front of the king to check for shield pawns
    int startFile = std::max(0, kingFile - 1);
    int endFile = std::min(7, kingFile + 1);
    int pawnRanks[2][3] = {
        {1, 2, 3},  // White: check ranks 1, 2, and 3 in front of king
        {6, 5, 4}   // Black: check ranks 6, 5, and 4 in front of king
    };
    
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);
    
    // Check pawns in the shield area
    for (int f = startFile; f <= endFile; f++) {
        for (int i = 0; i < 3; i++) {
            int r = pawnRanks[static_cast<int>(color)][i];
            int sq = r * 8 + f;
            if (pawns & (1ULL << sq)) {
                // Pawns closer to the king provide better protection
                count += KING_SHIELD_BONUS[i];
                break;  // Only count one pawn per file
            }
        }
    }
    
    return count;
}

int Evaluation::evaluateKingSafety(const chess::Board &board, int gamePhase) const
{
    // King safety is more important in the middlegame
    if (gamePhase < 64) {  // Endgame
        return 0;
    }
    
    int whiteScore = 0;
    int blackScore = 0;
    
    int whiteKingSq = chess::builtin::lsb(board.pieces(chess::PieceType::KING, chess::Color::WHITE));
    int blackKingSq = chess::builtin::lsb(board.pieces(chess::PieceType::KING, chess::Color::BLACK));
    
    // Evaluate pawn shield and storm for white king
    whiteScore += countPawnShield(board, whiteKingSq, chess::Color::WHITE);
    
    // Evaluate pawn shield and storm for black king
    blackScore += countPawnShield(board, blackKingSq, chess::Color::BLACK);
    
    // King exposure (open lines to the king)
    int whiteKingFile = whiteKingSq & 7;
    int blackKingFile = blackKingSq & 7;
    
    // Penalties for king on open or semi-open files
    if (isOpenFile(board, whiteKingFile)) {
        whiteScore -= 30;
    } else if (isSemiOpenFile(board, whiteKingFile, chess::Color::WHITE)) {
        whiteScore -= 15;
    }
    
    if (isOpenFile(board, blackKingFile)) {
        blackScore -= 30;
    } else if (isSemiOpenFile(board, blackKingFile, chess::Color::BLACK)) {
        blackScore -= 15;
    }
    
    // Check adjacent files too
    for (int offset = -1; offset <= 1; offset += 2) {
        int file = whiteKingFile + offset;
        if (file >= 0 && file <= 7) {
            if (isOpenFile(board, file)) {
                whiteScore -= 15;
            } else if (isSemiOpenFile(board, file, chess::Color::WHITE)) {
                whiteScore -= 7;
            }
        }
        
        file = blackKingFile + offset;
        if (file >= 0 && file <= 7) {
            if (isOpenFile(board, file)) {
                blackScore -= 15;
            } else if (isSemiOpenFile(board, file, chess::Color::BLACK)) {
                blackScore -= 7;
            }
        }
    }
    
    // Enemy piece tropism (penalty for enemy pieces close to king)
    chess::Bitboard whiteKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE);
    chess::Bitboard whiteBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE);
    chess::Bitboard whiteRooks = board.pieces(chess::PieceType::ROOK, chess::Color::WHITE);
    chess::Bitboard whiteQueens = board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE);
    
    chess::Bitboard blackKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK);
    chess::Bitboard blackBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK);
    chess::Bitboard blackRooks = board.pieces(chess::PieceType::ROOK, chess::Color::BLACK);
    chess::Bitboard blackQueens = board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK);
    
    // Check distance of enemy pieces to kings
    while (whiteQueens) {
        int sq = chess::builtin::poplsb(whiteQueens);
        int dist = getDistance(sq, blackKingSq);
        blackScore -= (8 - dist) * 5;  // Penalty based on closeness
    }
    
    while (blackQueens) {
        int sq = chess::builtin::poplsb(blackQueens);
        int dist = getDistance(sq, whiteKingSq);
        whiteScore -= (8 - dist) * 5;  // Penalty based on closeness
    }
    
    while (whiteRooks) {
        int sq = chess::builtin::poplsb(whiteRooks);
        int dist = getDistance(sq, blackKingSq);
        blackScore -= (8 - dist) * 3;
    }
    
    while (blackRooks) {
        int sq = chess::builtin::poplsb(blackRooks);
        int dist = getDistance(sq, whiteKingSq);
        whiteScore -= (8 - dist) * 3;
    }
    
    // Scale king safety by game phase (more important in middlegame)
    whiteScore = (whiteScore * gamePhase) / 256;
    blackScore = (blackScore * gamePhase) / 256;
    
    return whiteScore - blackScore;
}

int Evaluation::getDistance(int sq1, int sq2) const
{
    int file1 = sq1 & 7;
    int rank1 = sq1 >> 3;
    int file2 = sq2 & 7;
    int rank2 = sq2 >> 3;
    
    return std::max(std::abs(file1 - file2), std::abs(rank1 - rank2));
}

bool Evaluation::isPassed(const chess::Board &board, int sq, chess::Color color) const
{
    int file = sq & 7;
    int rank = sq >> 3;
    
    chess::Bitboard enemyPawns = board.pieces(chess::PieceType::PAWN, ~color);
    chess::Bitboard ahead = 0;
    
    if (color == chess::Color::WHITE) {
        for (int r = rank + 1; r < 8; r++) {
            if (file > 0) ahead |= 1ULL << (r * 8 + file - 1);
            ahead |= 1ULL << (r * 8 + file);
            if (file < 7) ahead |= 1ULL << (r * 8 + file + 1);
        }
    } else {
        for (int r = rank - 1; r >= 0; r--) {
            if (file > 0) ahead |= 1ULL << (r * 8 + file - 1);
            ahead |= 1ULL << (r * 8 + file);
            if (file < 7) ahead |= 1ULL << (r * 8 + file + 1);
        }
    }
    
    return !(ahead & enemyPawns);
}

bool Evaluation::isIsolated(const chess::Board &board, int sq, chess::Color color) const
{
    int file = sq & 7;
    chess::Bitboard adjacentFiles = 0;
    
    if (file > 0) adjacentFiles |= chess::attacks::MASK_FILE[file - 1];
    if (file < 7) adjacentFiles |= chess::attacks::MASK_FILE[file + 1];
    
    return !(board.pieces(chess::PieceType::PAWN, color) & adjacentFiles);
}

bool Evaluation::isDoubled(const chess::Board &board, int sq, chess::Color color) const
{
    int file = sq & 7;
    chess::Bitboard fileMask = chess::attacks::MASK_FILE[file];
    
    // Count pawns on the same file
    return chess::builtin::popcount(board.pieces(chess::PieceType::PAWN, color) & fileMask) > 1;
}

bool Evaluation::isConnected(const chess::Board &board, int sq, chess::Color color) const
{
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);
    int file = sq & 7;
    int rank = sq >> 3;
    
    chess::Bitboard pawnSquare = 1ULL << sq;
    chess::Bitboard adjacent = 0;
    
    // Check if there are pawns adjacent to this one (diagonally or on the same rank)
    if (file > 0) {
        // Left
        adjacent |= pawnSquare >> 1;
        if (color == chess::Color::WHITE && rank > 0)
            adjacent |= pawnSquare >> 9; // Diagonal down-left
        else if (color == chess::Color::BLACK && rank < 7)
            adjacent |= pawnSquare << 7; // Diagonal up-left
    }
    
    if (file < 7) {
        // Right
        adjacent |= pawnSquare << 1;
        if (color == chess::Color::WHITE && rank > 0)
            adjacent |= pawnSquare >> 7; // Diagonal down-right
        else if (color == chess::Color::BLACK && rank < 7)
            adjacent |= pawnSquare << 9; // Diagonal up-right
    }
    
    return (pawns & adjacent) != 0;
}

int Evaluation::evaluateThreats(const chess::Board &board) const
{
    int whiteScore = 0;
    int blackScore = 0;
    
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
    
    chess::Bitboard occupied = board.occ();
    chess::Bitboard whiteOcc = board.us(chess::Color::WHITE);
    chess::Bitboard blackOcc = board.us(chess::Color::BLACK);
    
    // Evaluate pawn threats
    chess::Bitboard whitePawnAttacks = 0;
    chess::Bitboard whiteTemp = whitePawns;
    while (whiteTemp) {
        int sq = chess::builtin::poplsb(whiteTemp);
        whitePawnAttacks |= chess::attacks::pawn(chess::Color::WHITE, static_cast<chess::Square>(sq));
    }
    
    chess::Bitboard blackPawnAttacks = 0;
    chess::Bitboard blackTemp = blackPawns;
    while (blackTemp) {
        int sq = chess::builtin::poplsb(blackTemp);
        blackPawnAttacks |= chess::attacks::pawn(chess::Color::BLACK, static_cast<chess::Square>(sq));
    }
    
    // Minor pieces attacked by enemy pawns
    int whitePiecesThreatened = chess::builtin::popcount(blackPawnAttacks & (whiteKnights | whiteBishops));
    int blackPiecesThreatened = chess::builtin::popcount(whitePawnAttacks & (blackKnights | blackBishops));
    
    whiteScore -= whitePiecesThreatened * 20;
    blackScore -= blackPiecesThreatened * 20;
    
    // Major pieces attacked by enemy pawns
    whitePiecesThreatened = chess::builtin::popcount(blackPawnAttacks & (whiteRooks | whiteQueens));
    blackPiecesThreatened = chess::builtin::popcount(whitePawnAttacks & (blackRooks | blackQueens));
    
    whiteScore -= whitePiecesThreatened * 40;
    blackScore -= blackPiecesThreatened * 40;
    
    // Hanging pieces (undefended pieces)
    chess::Bitboard whiteDefendedPieces = whitePawnAttacks;
    chess::Bitboard blackDefendedPieces = blackPawnAttacks;
    
    // Add knight attacks
    whiteTemp = whiteKnights;
    while (whiteTemp) {
        int sq = chess::builtin::poplsb(whiteTemp);
        whiteDefendedPieces |= chess::attacks::knight(static_cast<chess::Square>(sq));
    }
    
    blackTemp = blackKnights;
    while (blackTemp) {
        int sq = chess::builtin::poplsb(blackTemp);
        blackDefendedPieces |= chess::attacks::knight(static_cast<chess::Square>(sq));
    }
    
    // Add bishop attacks
    whiteTemp = whiteBishops;
    while (whiteTemp) {
        int sq = chess::builtin::poplsb(whiteTemp);
        whiteDefendedPieces |= chess::attacks::bishop(static_cast<chess::Square>(sq), occupied);
    }
    
    blackTemp = blackBishops;
    while (blackTemp) {
        int sq = chess::builtin::poplsb(blackTemp);
        blackDefendedPieces |= chess::attacks::bishop(static_cast<chess::Square>(sq), occupied);
    }
    
    // Add rook attacks
    whiteTemp = whiteRooks;
    while (whiteTemp) {
        int sq = chess::builtin::poplsb(whiteTemp);
        whiteDefendedPieces |= chess::attacks::rook(static_cast<chess::Square>(sq), occupied);
    }
    
    blackTemp = blackRooks;
    while (blackTemp) {
        int sq = chess::builtin::poplsb(blackTemp);
        blackDefendedPieces |= chess::attacks::rook(static_cast<chess::Square>(sq), occupied);
    }
    
    // Add queen attacks
    whiteTemp = whiteQueens;
    while (whiteTemp) {
        int sq = chess::builtin::poplsb(whiteTemp);
        whiteDefendedPieces |= chess::attacks::queen(static_cast<chess::Square>(sq), occupied);
    }
    
    blackTemp = blackQueens;
    while (blackTemp) {
        int sq = chess::builtin::poplsb(blackTemp);
        blackDefendedPieces |= chess::attacks::queen(static_cast<chess::Square>(sq), occupied);
    }
    
    // Count hanging pieces (excluding pawns and kings)
    chess::Bitboard whiteHangingPieces = (whiteKnights | whiteBishops | whiteRooks | whiteQueens) & ~whiteDefendedPieces;
    chess::Bitboard blackHangingPieces = (blackKnights | blackBishops | blackRooks | blackQueens) & ~blackDefendedPieces;
    
    // Penalize hanging pieces
    whiteScore -= chess::builtin::popcount(whiteHangingPieces) * 50;
    blackScore -= chess::builtin::popcount(blackHangingPieces) * 50;
    
    return whiteScore - blackScore;
}

int Evaluation::evaluatePieceCoordination(const chess::Board &board) const
{
    int whiteScore = 0;
    int blackScore = 0;
    
    chess::Bitboard whitePawns = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE);
    chess::Bitboard blackPawns = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);
    chess::Bitboard whiteKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE);
    chess::Bitboard blackKnights = board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK);
    chess::Bitboard whiteBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE);
    chess::Bitboard blackBishops = board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK);
    chess::Bitboard whiteRooks = board.pieces(chess::PieceType::ROOK, chess::Color::WHITE);
    chess::Bitboard blackRooks = board.pieces(chess::PieceType::ROOK, chess::Color::BLACK);
    
    // Evaluate connected/protected pawns
    chess::Bitboard whitePawnsCopy = whitePawns;
    while (whitePawnsCopy) {
        int sq = chess::builtin::poplsb(whitePawnsCopy);
        
        // Bonus for connected pawns
        if (isConnected(board, sq, chess::Color::WHITE)) {
            whiteScore += 10;
        }
        
        // Penalty for isolated pawns
        if (isIsolated(board, sq, chess::Color::WHITE)) {
            whiteScore -= 15;
        }
        
        // Penalty for doubled pawns
        if (isDoubled(board, sq, chess::Color::WHITE)) {
            whiteScore -= 10;
        }
    }
    
    chess::Bitboard blackPawnsCopy = blackPawns;
    while (blackPawnsCopy) {
        int sq = chess::builtin::poplsb(blackPawnsCopy);
        
        // Bonus for connected pawns
        if (isConnected(board, sq, chess::Color::BLACK)) {
            blackScore += 10;
        }
        
        // Penalty for isolated pawns
        if (isIsolated(board, sq, chess::Color::BLACK)) {
            blackScore -= 15;
        }
        
        // Penalty for doubled pawns
        if (isDoubled(board, sq, chess::Color::BLACK)) {
            blackScore -= 10;
        }
    }
    
    // Evaluate rook coordination (rooks on same rank or file)
    if (chess::builtin::popcount(whiteRooks) >= 2) {
        chess::Bitboard rooks = whiteRooks;
        int firstRook = chess::builtin::poplsb(rooks);
        int firstRookRank = firstRook >> 3;
        int firstRookFile = firstRook & 7;
        
        bool rookConnection = false;
        chess::Bitboard remainingRooks = rooks;
        
        while (remainingRooks) {
            int secondRook = chess::builtin::poplsb(remainingRooks);
            int secondRookRank = secondRook >> 3;
            int secondRookFile = secondRook & 7;
            
            // Connected on rank
            if (firstRookRank == secondRookRank) {
                rookConnection = true;
                break;
            }
            
            // Connected on file
            if (firstRookFile == secondRookFile) {
                rookConnection = true;
                break;
            }
        }
        
        if (rookConnection) {
            whiteScore += 15;
        }
    }
    
    if (chess::builtin::popcount(blackRooks) >= 2) {
        chess::Bitboard rooks = blackRooks;
        int firstRook = chess::builtin::poplsb(rooks);
        int firstRookRank = firstRook >> 3;
        int firstRookFile = firstRook & 7;
        
        bool rookConnection = false;
        chess::Bitboard remainingRooks = rooks;
        
        while (remainingRooks) {
            int secondRook = chess::builtin::poplsb(remainingRooks);
            int secondRookRank = secondRook >> 3;
            int secondRookFile = secondRook & 7;
            
            // Connected on rank
            if (firstRookRank == secondRookRank) {
                rookConnection = true;
                break;
            }
            
            // Connected on file
            if (firstRookFile == secondRookFile) {
                rookConnection = true;
                break;
            }
        }
        
        if (rookConnection) {
            blackScore += 15;
        }
    }
    
    // Knight outposts
    chess::Bitboard whiteKnightsCopy = whiteKnights;
    while (whiteKnightsCopy) {
        int sq = chess::builtin::poplsb(whiteKnightsCopy);
        int rank = sq >> 3;
        int file = sq & 7;
        
        // Knight on outpost (advanced, protected by pawn, can't be attacked by enemy pawn)
        if (rank >= 4) {  // On opponent's half
            chess::Bitboard attackDefenders = chess::attacks::pawn(chess::Color::WHITE, static_cast<chess::Square>(sq));
            bool protectedByPawn = (attackDefenders & whitePawns) != 0;
            
            // Check if it can be attacked by enemy pawns
            bool attackedByPawn = false;
            if (file > 0) {
                attackedByPawn |= blackPawns & (1ULL << (sq + 7));
            }
            if (file < 7) {
                attackedByPawn |= blackPawns & (1ULL << (sq + 9));
            }
            
            if (protectedByPawn && !attackedByPawn) {
                whiteScore += 20;  // Strong outpost
            }
        }
    }
    
    chess::Bitboard blackKnightsCopy = blackKnights;
    while (blackKnightsCopy) {
        int sq = chess::builtin::poplsb(blackKnightsCopy);
        int rank = sq >> 3;
        int file = sq & 7;
        
        // Knight on outpost (advanced, protected by pawn, can't be attacked by enemy pawn)
        if (rank <= 3) {  // On opponent's half
            chess::Bitboard attackDefenders = chess::attacks::pawn(chess::Color::BLACK, static_cast<chess::Square>(sq));
            bool protectedByPawn = (attackDefenders & blackPawns) != 0;
            
            // Check if it can be attacked by enemy pawns
            bool attackedByPawn = false;
            if (file > 0) {
                attackedByPawn |= whitePawns & (1ULL << (sq - 9));
            }
            if (file < 7) {
                attackedByPawn |= whitePawns & (1ULL << (sq - 7));
            }
            
            if (protectedByPawn && !attackedByPawn) {
                blackScore += 20;  // Strong outpost
            }
        }
    }
    
    // Evaluate bishop and knight coordination
    // Bishops work well on open board, knights on closed positions
    int whitePawnCount = chess::builtin::popcount(whitePawns);
    int blackPawnCount = chess::builtin::popcount(blackPawns);
    int totalPawns = whitePawnCount + blackPawnCount;
    
    if (totalPawns <= 8) {  // Open position
        // Bishops are stronger in open positions
        whiteScore += chess::builtin::popcount(whiteBishops) * 10;
        blackScore += chess::builtin::popcount(blackBishops) * 10;
    } else if (totalPawns >= 12) {  // Closed position
        // Knights are stronger in closed positions
        whiteScore += chess::builtin::popcount(whiteKnights) * 10;
        blackScore += chess::builtin::popcount(blackKnights) * 10;
    }
    
    return whiteScore - blackScore;
}