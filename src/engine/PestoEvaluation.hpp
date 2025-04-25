#ifndef PESTO_EVALUATION_HPP
#define PESTO_EVALUATION_HPP

#include "../chess.hpp"
#include <array>
#include <vector>
#include <map>
#include <random>

class PestoEvaluation
{
public:
    PestoEvaluation();
    ~PestoEvaluation() = default;

    // Main evaluation function
    int evaluate(const chess::Board &board);

    // Piece types enum for internal use
    enum PieceIndex
    {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
        NONE
    };

private:
    // Piece values for middlegame and endgame
    static constexpr int MG_PAWN_VALUE = 82;
    static constexpr int MG_KNIGHT_VALUE = 337;
    static constexpr int MG_BISHOP_VALUE = 365;
    static constexpr int MG_ROOK_VALUE = 477;
    static constexpr int MG_QUEEN_VALUE = 1025;
    static constexpr int MG_KING_VALUE = 0;

    static constexpr int EG_PAWN_VALUE = 94;
    static constexpr int EG_KNIGHT_VALUE = 281;
    static constexpr int EG_BISHOP_VALUE = 297;
    static constexpr int EG_ROOK_VALUE = 512;
    static constexpr int EG_QUEEN_VALUE = 936;
    static constexpr int EG_KING_VALUE = 0;

    static constexpr int PHASE_PAWN_VALUE = 0;
    static constexpr int PHASE_KNIGHT_VALUE = 1;
    static constexpr int PHASE_BISHOP_VALUE = 1;
    static constexpr int PHASE_ROOK_VALUE = 2;
    static constexpr int PHASE_QUEEN_VALUE = 4;
    static constexpr int TOTAL_PHASE = 24; // 16 for pawns + 4 for knights + 4 for bishops + 4 for rooks + 8 for queens

    // Arrays to hold piece values for both game phases
    std::array<int, 6> mg_value;
    std::array<int, 6> eg_value;

    // Piece-square tables for middlegame and endgame
    std::array<std::array<int, 64>, 6> mg_pesto_table;
    std::array<std::array<int, 64>, 6> eg_pesto_table;

    // Initialize the piece-square tables
    void initializeTables();

    // Convert chess::PieceType to our internal PieceIndex
    PieceIndex pieceTypeToIndex(chess::PieceType pt) const;

    int evaluatePassedPawns(const chess::Board &board, chess::Color color);
    int evaluatePawnStructure(const chess::Board &board, chess::Color color);
    int evaluateKingSafety(const chess::Board &board, chess::Color color);

    bool isPawnPassed(const chess::Board &board, chess::Square pawnSq, chess::Color color) const;
    bool isPawnIsolated(const chess::Board &board, chess::Square pawnSq, chess::Color color) const;
    bool isPawnDoubled(const chess::Board &board, chess::Square pawnSq, chess::Color color) const;
    int kingAttackersCount(const chess::Board &board, chess::Square kingSq, chess::Color attackerColor) const;

    // Constants for new evaluation components
    static constexpr int PASSED_PAWN_BONUS[8] = {0, 10, 20, 40, 60, 100, 150, 0};
    static constexpr int ISOLATED_PAWN_PENALTY = -15;
    static constexpr int DOUBLED_PAWN_PENALTY = -10;
    static constexpr int PAWN_SHIELD_BONUS = 10;
    static constexpr int KING_SAFETY_ATTACK_WEIGHT = -10;
};

#endif // PESTO_EVALUATION_HPP