#include "../chess.hpp"
#include "ChessEngine.hpp"

#include <algorithm>
#include <cmath>

namespace pesto {

// Piece values for midgame and endgame
const int MG_PAWN_VALUE = 82;
const int MG_KNIGHT_VALUE = 337;
const int MG_BISHOP_VALUE = 365;
const int MG_ROOK_VALUE = 477;
const int MG_QUEEN_VALUE = 1025;
const int MG_KING_VALUE = 0;

const int EG_PAWN_VALUE = 94;
const int EG_KNIGHT_VALUE = 281;
const int EG_BISHOP_VALUE = 297;
const int EG_ROOK_VALUE = 512;
const int EG_QUEEN_VALUE = 936;
const int EG_KING_VALUE = 0;

// Array to store the values of pieces in midgame and endgame
const int MG_PIECE_VALUES[6] = {
    MG_PAWN_VALUE, MG_KNIGHT_VALUE, MG_BISHOP_VALUE, 
    MG_ROOK_VALUE, MG_QUEEN_VALUE, MG_KING_VALUE
};
const int EG_PIECE_VALUES[6] = {
    EG_PAWN_VALUE, EG_KNIGHT_VALUE, EG_BISHOP_VALUE, 
    EG_ROOK_VALUE, EG_QUEEN_VALUE, EG_KING_VALUE
};

// Piece-square tables for midgame
const int MG_PAWN_TABLE[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0
};

const int MG_KNIGHT_TABLE[64] = {
    -169, -50, -35, -29, -29, -35, -50, -169,
     -50, -25,  -10,  -5,  -5, -10, -25,  -50,
     -35, -10,   0,   5,   5,   0, -10,  -35,
     -29,  -5,   5,  10,  10,   5,  -5,  -29,
     -29,  -5,   5,  10,  10,   5,  -5,  -29,
     -35, -10,   0,   5,   5,   0, -10,  -35,
     -50, -25, -10,  -5,  -5, -10, -25,  -50,
    -169, -50, -35, -29, -29, -35, -50, -169
};

const int MG_BISHOP_TABLE[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

const int MG_ROOK_TABLE[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

const int MG_QUEEN_TABLE[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10,   0,   5,  0,  0,   0,   0, -10,
    -10,   5,   5,  5,  5,   5,   0, -10,
      0,   0,   5,  5,  5,   5,   0,  -5,
     -5,   0,   5,  5,  5,   5,   0,  -5,
    -10,   0,   5,  5,  5,   5,   0, -10,
    -10,   0,   0,  0,  0,   0,   0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

const int MG_KING_TABLE[64] = {
     20,  30,  10,   0,   0,  10,  30,  20,
     20,  20,   0,   0,   0,   0,  20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
};

// Piece-square tables for endgame
const int EG_PAWN_TABLE[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0
};

const int EG_KNIGHT_TABLE[64] = {
    -58, -38, -13, -28, -28, -13, -38, -58,
    -30, -13,   0, -10, -10,   0, -13, -30,
    -15,   8,  20,   4,   4,  20,   8, -15,
      0,  23,  23,  35,  35,  23,  23,   0,
      3,  23,  20,  20,  20,  20,  23,   3,
    -12,  10,  20,  16,  16,  20,  10, -12,
    -12,   0,   1,  10,  10,   1,   0, -12,
    -43, -21, -18, -18, -18, -18, -21, -43
};

const int EG_BISHOP_TABLE[64] = {
    -14, -21, -11,  -8,  -8, -11, -21, -14,
     -8,  -4,  -1,  -1,  -1,  -1,  -4,  -8,
     -2,   0,   6,   5,   5,   6,   0,  -2,
     -4,   5,   9,  10,  10,   9,   5,  -4,
     -4,   3,   5,  10,  10,   5,   3,  -4,
     -1,   1,   5,   5,   5,   5,   1,  -1,
     -8,  -1,  -1,  -1,  -1,  -1,  -1,  -8,
    -14, -21, -11,  -8,  -8, -11, -21, -14
};

const int EG_ROOK_TABLE[64] = {
     13, 10, 18, 15, 15, 18, 10, 13,
     11, 13, 13, 11, 11, 13, 13, 11,
      7,  7,  7,  5,  5,  7,  7,  7,
      0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,
     -3, -3, -3, -3, -3, -3, -3, -3,
     -3,  0,  0,  0,  0,  0,  0, -3,
      3, -1, -1,  5,  5, -1, -1,  3
};

const int EG_QUEEN_TABLE[64] = {
     -9,  22,  22,  27,  27,  22,  22,  -9,
    -17,  20,  32,  41,  41,  32,  20, -17,
    -20,   6,   9,  49,  49,   9,   6, -20,
      9,  22,  22,  35,  35,  22,  22,   9,
     -3,  27,  27,  35,  35,  27,  27,  -3,
     -9,  22,  22,  35,  35,  22,  22,  -9,
    -30,  -5,  27,  12,  12,  27,  -5, -30,
    -74, -35, -18, -18, -18, -18, -35, -74
};

const int EG_KING_TABLE[64] = {
    -74, -35, -18, -18, -18, -18, -35, -74,
    -12,  17,  14,  17,  17,  14,  17, -12,
     10,  17,  23,  15,  15,  23,  17,  10,
      4,  15,  15,  12,  12,  15,  15,   4,
     -9,  -6,   3,   3,   3,   3,  -6,  -9,
    -11,  -3,  -1,   1,   1,  -1,  -3, -11,
    -71, -15,  -3,  -1,  -1,  -3, -15, -71,
    -74, -12,  -7,  -8,  -8,  -7, -12, -74
};

// Arrays to store all piece-square tables for easy access
const int* MG_TABLES[6] = {
    MG_PAWN_TABLE, MG_KNIGHT_TABLE, MG_BISHOP_TABLE,
    MG_ROOK_TABLE, MG_QUEEN_TABLE, MG_KING_TABLE
};

const int* EG_TABLES[6] = {
    EG_PAWN_TABLE, EG_KNIGHT_TABLE, EG_BISHOP_TABLE,
    EG_ROOK_TABLE, EG_QUEEN_TABLE, EG_KING_TABLE
};

// Piece count for game phase calculation
const int GAME_PHASE_INC[6] = { 0, 1, 1, 2, 4, 0 };
const int GAME_PHASE_MAX = 24; // 16 pieces without pawns and kings

// Function to evaluate a chess position using Pesto's evaluation function
int evaluate(const chess::Board& board) {
    int mg[2] = { 0, 0 }; // MG scores for white and black
    int eg[2] = { 0, 0 }; // EG scores for white and black
    int gamePhase = 0;

    // Loop through all squares
    for (int sq = 0; sq < 64; ++sq) {
        chess::Square square = static_cast<chess::Square>(sq);
        chess::Piece piece = board.at(square);
        
        if (piece == chess::Piece::NONE) {
            continue;
        }

        // Determine piece type and color
        chess::PieceType type = chess::utils::typeOfPiece(piece);
        chess::Color color = board.color(piece);
        int colorIdx = static_cast<int>(color);

        // Get piece value and position score
        int pieceValue = MG_PIECE_VALUES[static_cast<int>(type)];
        
        // For square tables, we need to flip the square for black pieces
        int position = sq;
        if (color == chess::Color::BLACK) {
            position = sq ^ 56; // Flip vertically
        }

        // Update midgame and endgame scores
        mg[colorIdx] += pieceValue + MG_TABLES[static_cast<int>(type)][position];
        eg[colorIdx] += EG_PIECE_VALUES[static_cast<int>(type)] + EG_TABLES[static_cast<int>(type)][position];

        // Update game phase based on piece
        gamePhase += GAME_PHASE_INC[static_cast<int>(type)];
    }

    // Calculate total midgame and endgame scores from white's perspective
    int mgScore = mg[static_cast<int>(chess::Color::WHITE)] - mg[static_cast<int>(chess::Color::BLACK)];
    int egScore = eg[static_cast<int>(chess::Color::WHITE)] - eg[static_cast<int>(chess::Color::BLACK)];

    // Taper evaluation based on game phase
    gamePhase = std::min(gamePhase, GAME_PHASE_MAX);
    int finalScore = (mgScore * gamePhase + egScore * (GAME_PHASE_MAX - gamePhase)) / GAME_PHASE_MAX;

    // Return the score from the perspective of the side to move
    return (board.sideToMove() == chess::Color::WHITE) ? finalScore : -finalScore;
}

} // namespace pesto

// Export the evaluation function to be used by the chess engine
int ChessEngine::evaluatePosition(const chess::Board& board) {
    return pesto::evaluate(board);
}

