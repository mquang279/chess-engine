#include "PestoEvaluation.hpp"
#include <algorithm>
#include <cmath>

// Pesto's piece-square tables for middlegame
const int MG_PAWN_TABLE[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, 0, 1, 3, 3, 1, 0, -3,
    -5, -1, -1, 0, 0, -1, -1, -5,
    -7, -3, -2, 1, 1, -2, -3, -7,
    -10, -5, -4, 0, 0, -4, -5, -10,
    -15, -8, -7, -4, -4, -7, -8, -15,
    -20, -12, -10, -8, -8, -10, -12, -20,
    0, 0, 0, 0, 0, 0, 0, 0};

const int MG_KNIGHT_TABLE[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};

const int MG_BISHOP_TABLE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 5, 5, 5, 5, -10,
    -10, 0, 5, 0, 0, 5, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};

const int MG_ROOK_TABLE[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0};

const int MG_QUEEN_TABLE[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20};

const int MG_KING_TABLE[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20, 0, 0, 0, 0, 20, 20,
    20, 30, 10, 0, 0, 10, 30, 20};

// Pesto's piece-square tables for endgame
const int EG_PAWN_TABLE[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0};

const int EG_KNIGHT_TABLE[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};

const int EG_BISHOP_TABLE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 5, 5, 5, 5, -10,
    -10, 0, 5, 0, 0, 5, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};

const int EG_ROOK_TABLE[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0};

const int EG_QUEEN_TABLE[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10, 0, 0, -10, -20, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -30, 0, 0, 0, 0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50};

const int EG_KING_TABLE[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10, 0, 0, -10, -20, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -30, 0, 0, 0, 0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50};

PestoEvaluation::PestoEvaluation()
{
    // Initialize piece values
    mg_value = {MG_PAWN_VALUE, MG_KNIGHT_VALUE, MG_BISHOP_VALUE, MG_ROOK_VALUE, MG_QUEEN_VALUE, MG_KING_VALUE};
    eg_value = {EG_PAWN_VALUE, EG_KNIGHT_VALUE, EG_BISHOP_VALUE, EG_ROOK_VALUE, EG_QUEEN_VALUE, EG_KING_VALUE};

    initializeTables();
}

void PestoEvaluation::initializeTables()
{
    // Initialize the middlegame tables
    for (int i = 0; i < 64; i++)
    {
        mg_pesto_table[PAWN][i] = MG_PAWN_TABLE[i];
        mg_pesto_table[KNIGHT][i] = MG_KNIGHT_TABLE[i];
        mg_pesto_table[BISHOP][i] = MG_BISHOP_TABLE[i];
        mg_pesto_table[ROOK][i] = MG_ROOK_TABLE[i];
        mg_pesto_table[QUEEN][i] = MG_QUEEN_TABLE[i];
        mg_pesto_table[KING][i] = MG_KING_TABLE[i];

        // Initialize the endgame tables
        eg_pesto_table[PAWN][i] = EG_PAWN_TABLE[i];
        eg_pesto_table[KNIGHT][i] = EG_KNIGHT_TABLE[i];
        eg_pesto_table[BISHOP][i] = EG_BISHOP_TABLE[i];
        eg_pesto_table[ROOK][i] = EG_ROOK_TABLE[i];
        eg_pesto_table[QUEEN][i] = EG_QUEEN_TABLE[i];
        eg_pesto_table[KING][i] = EG_KING_TABLE[i];
    }
}

PestoEvaluation::PieceIndex PestoEvaluation::pieceTypeToIndex(chess::PieceType pt) const
{
    switch (pt)
    {
    case chess::PieceType::PAWN:
        return PAWN;
    case chess::PieceType::KNIGHT:
        return KNIGHT;
    case chess::PieceType::BISHOP:
        return BISHOP;
    case chess::PieceType::ROOK:
        return ROOK;
    case chess::PieceType::QUEEN:
        return QUEEN;
    case chess::PieceType::KING:
        return KING;
    default:
        return NONE;
    }
}

int PestoEvaluation::evaluate(const chess::Board &board)
{
    int mg_score = 0;
    int eg_score = 0;
    int phase = 0;

    // Side to move
    chess::Color side_to_move = board.sideToMove();

    // Evaluate all pieces
    for (int pt = static_cast<int>(chess::PieceType::PAWN);
         pt <= static_cast<int>(chess::PieceType::KING); ++pt)
    {
        chess::PieceType piece_type = static_cast<chess::PieceType>(pt);
        PieceIndex piece_idx = pieceTypeToIndex(piece_type);

        // Evaluate white pieces
        chess::Bitboard white_pieces = board.pieces(piece_type, chess::Color::WHITE);
        while (white_pieces)
        {
            chess::Square sq = chess::builtin::poplsb(white_pieces);
            int sq_idx = static_cast<int>(sq);

            // Material score
            mg_score += mg_value[piece_idx];
            eg_score += eg_value[piece_idx];

            // Piece-square table score
            mg_score += mg_pesto_table[piece_idx][sq_idx];
            eg_score += eg_pesto_table[piece_idx][sq_idx];

            // Update game phase
            if (piece_type == chess::PieceType::KNIGHT ||
                piece_type == chess::PieceType::BISHOP)
            {
                phase += 1;
            }
            else if (piece_type == chess::PieceType::ROOK)
            {
                phase += 2;
            }
            else if (piece_type == chess::PieceType::QUEEN)
            {
                phase += 4;
            }
        }

        // Evaluate black pieces
        chess::Bitboard black_pieces = board.pieces(piece_type, chess::Color::BLACK);
        while (black_pieces)
        {
            chess::Square sq = chess::builtin::poplsb(black_pieces);
            // Mirror square vertically for black pieces
            int sq_idx = static_cast<int>(sq) ^ 56;

            // Material score (negative for black)
            mg_score -= mg_value[piece_idx];
            eg_score -= eg_value[piece_idx];

            // Piece-square table score (negative for black)
            mg_score -= mg_pesto_table[piece_idx][sq_idx];
            eg_score -= eg_pesto_table[piece_idx][sq_idx];

            // Update game phase
            if (piece_type == chess::PieceType::KNIGHT ||
                piece_type == chess::PieceType::BISHOP)
            {
                phase += 1;
            }
            else if (piece_type == chess::PieceType::ROOK)
            {
                phase += 2;
            }
            else if (piece_type == chess::PieceType::QUEEN)
            {
                phase += 4;
            }
        }
    }

    // Evaluate pawn structure and passed pawns - weight more in endgame
    int white_pawn_structure = evaluatePawnStructure(board, chess::Color::WHITE);
    int black_pawn_structure = evaluatePawnStructure(board, chess::Color::BLACK);
    int white_passed_pawns = evaluatePassedPawns(board, chess::Color::WHITE);
    int black_passed_pawns = evaluatePassedPawns(board, chess::Color::BLACK);

    // Pawn structure evaluation - weight less in middlegame, more in endgame
    mg_score += (white_pawn_structure - black_pawn_structure) / 2;
    eg_score += (white_pawn_structure - black_pawn_structure);

    // Passed pawn evaluation - weight more in endgame
    mg_score += (white_passed_pawns - black_passed_pawns) / 2;
    eg_score += (white_passed_pawns - black_passed_pawns) * 2;

    // King safety evaluation - more important in middlegame than endgame
    int white_king_safety = evaluateKingSafety(board, chess::Color::WHITE);
    int black_king_safety = evaluateKingSafety(board, chess::Color::BLACK);
    mg_score += (white_king_safety - black_king_safety);
    eg_score += (white_king_safety - black_king_safety) / 2;

    // Ensure phase value is within bounds
    phase = std::min(phase, TOTAL_PHASE);

    // Interpolate between middlegame and endgame scores
    int score = (mg_score * phase + eg_score * (TOTAL_PHASE - phase)) / TOTAL_PHASE;

    // Return score from the perspective of the side to move
    return side_to_move == chess::Color::WHITE ? score : -score;
}

// Implementation of the helper methods for pawn evaluation
bool PestoEvaluation::isPawnPassed(const chess::Board &board, chess::Square pawnSq, chess::Color color) const
{
    int sq = static_cast<int>(pawnSq);
    int file = sq % 8; // Square's file (0-7)
    int rank = sq / 8; // Square's rank (0-7)

    // Direction to check depends on pawn color
    int direction = (color == chess::Color::WHITE) ? 1 : -1;
    int startRank = (color == chess::Color::WHITE) ? rank + 1 : 0;
    int endRank = (color == chess::Color::WHITE) ? 7 : rank - 1;

    // Check if there are any enemy pawns in front or diagonally in front
    for (int r = startRank; r <= endRank; r++)
    {
        // Same file in front
        if (file >= 0 && file < 8)
        {
            chess::Square frontSq = static_cast<chess::Square>(r * 8 + file);
            if (board.pieces(chess::PieceType::PAWN, ~color) & (1ULL << frontSq))
            {
                return false;
            }
        }

        // Left diagonal file if not on a-file
        if (file > 0)
        {
            chess::Square leftSq = static_cast<chess::Square>(r * 8 + file - 1);
            if (board.pieces(chess::PieceType::PAWN, ~color) & (1ULL << leftSq))
            {
                return false;
            }
        }

        // Right diagonal file if not on h-file
        if (file < 7)
        {
            chess::Square rightSq = static_cast<chess::Square>(r * 8 + file + 1);
            if (board.pieces(chess::PieceType::PAWN, ~color) & (1ULL << rightSq))
            {
                return false;
            }
        }
    }

    return true;
}

bool PestoEvaluation::isPawnIsolated(const chess::Board &board, chess::Square pawnSq, chess::Color color) const
{
    int sq = static_cast<int>(pawnSq);
    int file = sq % 8; // Square's file (0-7)

    // Check adjacent files for friendly pawns
    chess::Bitboard friendlyPawns = board.pieces(chess::PieceType::PAWN, color);
    chess::Bitboard fileMasks[8] = {0x0101010101010101ULL, 0x0202020202020202ULL,
                                    0x0404040404040404ULL, 0x0808080808080808ULL,
                                    0x1010101010101010ULL, 0x2020202020202020ULL,
                                    0x4040404040404040ULL, 0x8080808080808080ULL};

    // Check left file if not on a-file
    if (file > 0 && !(friendlyPawns & fileMasks[file - 1]))
    {
        // Check right file if not on h-file
        if (file < 7)
        {
            return !(friendlyPawns & fileMasks[file + 1]);
        }
        return true; // On h-file with no pawns on g-file
    }

    // Check right file if not on h-file
    if (file < 7)
    {
        return !(friendlyPawns & fileMasks[file + 1]);
    }

    return true; // On a-file with no pawns on b-file
}

bool PestoEvaluation::isPawnDoubled(const chess::Board &board, chess::Square pawnSq, chess::Color color) const
{
    int sq = static_cast<int>(pawnSq);
    int file = sq % 8; // Square's file (0-7)

    // Check same file for multiple pawns
    chess::Bitboard friendlyPawns = board.pieces(chess::PieceType::PAWN, color);
    chess::Bitboard fileMask = 0x0101010101010101ULL << file; // Mask for the current file

    return chess::builtin::popcount(friendlyPawns & fileMask) > 1;
}

int PestoEvaluation::kingAttackersCount(const chess::Board &board, chess::Square kingSq, chess::Color attackerColor) const
{
    int attackers = 0;
    chess::Bitboard kingArea = chess::attacks::king(kingSq);
    kingArea |= (1ULL << kingSq); // Include the king's square

    // Check if any enemy pieces attack the king area
    chess::Bitboard knights = board.pieces(chess::PieceType::KNIGHT, attackerColor) & chess::attacks::knight(kingSq);
    attackers += chess::builtin::popcount(knights);

    chess::Bitboard bishops = board.pieces(chess::PieceType::BISHOP, attackerColor);
    chess::Bitboard queens = board.pieces(chess::PieceType::QUEEN, attackerColor);
    chess::Bitboard bishopAttacks = chess::attacks::bishop(kingSq, board.occ()) & (bishops | queens);
    attackers += chess::builtin::popcount(bishopAttacks);

    chess::Bitboard rooks = board.pieces(chess::PieceType::ROOK, attackerColor);
    chess::Bitboard rookAttacks = chess::attacks::rook(kingSq, board.occ()) & (rooks | queens);
    attackers += chess::builtin::popcount(rookAttacks);

    // Pawns attacking the king area
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, attackerColor);
    if (attackerColor == chess::Color::WHITE)
    {
        // White pawns attack upward-left and upward-right
        chess::Bitboard pawnAttacks = ((pawns << 7) & ~0x8080808080808080ULL) | // Left attacks
                                      ((pawns << 9) & ~0x0101010101010101ULL);  // Right attacks
        attackers += chess::builtin::popcount(pawnAttacks & kingArea);
    }
    else
    {
        // Black pawns attack downward-left and downward-right
        chess::Bitboard pawnAttacks = ((pawns >> 9) & ~0x8080808080808080ULL) | // Left attacks
                                      ((pawns >> 7) & ~0x0101010101010101ULL);  // Right attacks
        attackers += chess::builtin::popcount(pawnAttacks & kingArea);
    }

    return attackers;
}

// Implementation of the evaluation components
int PestoEvaluation::evaluatePassedPawns(const chess::Board &board, chess::Color color)
{
    int score = 0;
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);

    while (pawns)
    {
        chess::Square sq = chess::builtin::poplsb(pawns);

        if (isPawnPassed(board, sq, color))
        {
            int rank = static_cast<int>(sq) / 8;
            // Convert rank for black pawns (7 - rank)
            int adjustedRank = (color == chess::Color::WHITE) ? rank : 7 - rank;
            score += PASSED_PAWN_BONUS[adjustedRank];

            // Extra bonus if passed pawn is protected
            chess::Bitboard pawnDefenders = board.pieces(chess::PieceType::PAWN, color);
            if (color == chess::Color::WHITE)
            {
                // Check if any white pawn defends this passed pawn
                chess::Bitboard defenders = ((pawnDefenders >> 7) & ~0x0101010101010101ULL) | // Right attacks
                                            ((pawnDefenders >> 9) & ~0x8080808080808080ULL);  // Left attacks
                if (defenders & (1ULL << sq))
                {
                    score += PASSED_PAWN_BONUS[adjustedRank] / 2;
                }
            }
            else
            {
                // Check if any black pawn defends this passed pawn
                chess::Bitboard defenders = ((pawnDefenders << 7) & ~0x8080808080808080ULL) | // Left attacks
                                            ((pawnDefenders << 9) & ~0x0101010101010101ULL);  // Right attacks
                if (defenders & (1ULL << sq))
                {
                    score += PASSED_PAWN_BONUS[adjustedRank] / 2;
                }
            }
        }
    }

    return score;
}

int PestoEvaluation::evaluatePawnStructure(const chess::Board &board, chess::Color color)
{
    int score = 0;
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);

    while (pawns)
    {
        chess::Square sq = chess::builtin::poplsb(pawns);

        if (isPawnIsolated(board, sq, color))
        {
            score += ISOLATED_PAWN_PENALTY;
        }

        if (isPawnDoubled(board, sq, color))
        {
            score += DOUBLED_PAWN_PENALTY;
        }
    }

    return score;
}

int PestoEvaluation::evaluateKingSafety(const chess::Board &board, chess::Color color)
{
    int score = 0;

    // Get the king square
    chess::Square kingSq = board.kingSq(color);
    int kingFile = static_cast<int>(kingSq) % 8;
    int kingRank = static_cast<int>(kingSq) / 8;

    // Check pawn shield in front of the king (important for castled kings)
    chess::Bitboard pawns = board.pieces(chess::PieceType::PAWN, color);
    chess::Bitboard kingZone = chess::attacks::king(kingSq);
    chess::Bitboard pawnShield = pawns & kingZone;

    // Add bonus for each pawn in the king's shield
    score += chess::builtin::popcount(pawnShield) * PAWN_SHIELD_BONUS;

    // Penalize for enemy pieces attacking king area
    int attackers = kingAttackersCount(board, kingSq, ~color);
    score += attackers * KING_SAFETY_ATTACK_WEIGHT;

    // Bonus for castled king or king in a safe corner
    bool kingCastled = false;
    if (color == chess::Color::WHITE)
    {
        kingCastled = (kingFile <= 2 && kingRank == 0) || (kingFile >= 6 && kingRank == 0);
    }
    else
    {
        kingCastled = (kingFile <= 2 && kingRank == 7) || (kingFile >= 6 && kingRank == 7);
    }

    if (kingCastled)
    {
        score += 15; // Bonus for castled king
    }

    return score;
}