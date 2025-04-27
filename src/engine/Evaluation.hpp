#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include "../chess.hpp"
#include <array>

class Evaluation
{
public:
    Evaluation() = default;

    int evaluate(const chess::Board &board) const;

private:
    // Material values (in centipawns)
    static constexpr std::array<int, 6> PIECE_VALUES = {100, 320, 330, 500, 900, 20000};

    // Mobility bonuses per piece type (Knight, Bishop, Rook, Queen)
    static constexpr std::array<int, 4> MOBILITY_BONUS = {4, 5, 3, 2};

    // Piece-Square Tables
    static constexpr std::array<int, 64> PAWN_PST = {
        0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5, 5, 10, 25, 25, 10, 5, 5,
        0, 0, 0, 20, 20, 0, 0, 0,
        5, -5, -10, 0, 0, -10, -5, 5,
        5, 10, 10, -20, -20, 10, 10, 5,
        0, 0, 0, 0, 0, 0, 0, 0};

    static constexpr std::array<int, 64> KNIGHT_PST = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -30, 0, 10, 15, 15, 10, 0, -30,
        -30, 5, 15, 20, 20, 15, 5, -30,
        -30, 0, 15, 20, 20, 15, 0, -30,
        -30, 5, 10, 15, 15, 10, 5, -30,
        -40, -20, 0, 5, 5, 0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50};

    static constexpr std::array<int, 64> BISHOP_PST = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 10, 10, 10, 10, 0, -10,
        -10, 5, 5, 10, 10, 5, 5, -10,
        -10, 0, 5, 10, 10, 5, 0, -10,
        -10, 10, 10, 10, 10, 10, 10, -10,
        -10, 5, 0, 0, 0, 0, 5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20};

    static constexpr std::array<int, 64> ROOK_PST = {
        0, 0, 0, 0, 0, 0, 0, 0,
        5, 10, 10, 10, 10, 10, 10, 5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        -5, 0, 0, 0, 0, 0, 0, -5,
        0, 0, 0, 5, 5, 0, 0, 0};

    static constexpr std::array<int, 64> QUEEN_PST = {
        -20, -10, -10, -5, -5, -10, -10, -20,
        -10, 0, 0, 0, 0, 0, 0, -10,
        -10, 0, 5, 5, 5, 5, 0, -10,
        -5, 0, 5, 5, 5, 5, 0, -5,
        0, 0, 5, 5, 5, 5, 0, -5,
        -10, 5, 5, 5, 5, 5, 0, -10,
        -10, 0, 5, 0, 0, 0, 0, -10,
        -20, -10, -10, -5, -5, -10, -10, -20};

    static constexpr std::array<int, 64> KING_MIDDLE_PST = {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        20, 20, 0, 0, 0, 0, 20, 20,
        20, 30, 10, 0, 0, 10, 30, 20};

    static constexpr std::array<int, 64> KING_END_PST = {
        -50, -40, -30, -20, -20, -30, -40, -50,
        -30, -20, -10, 0, 0, -10, -20, -30,
        -30, -10, 20, 30, 30, 20, -10, -30,
        -30, -10, 30, 40, 40, 30, -10, -30,
        -30, -10, 30, 40, 40, 30, -10, -30,
        -30, -10, 20, 30, 30, 20, -10, -30,
        -30, -30, 0, 0, 0, 0, -30, -30,
        -50, -30, -30, -30, -30, -30, -30, -50};

    // King safety shield pattern for pawns in front of king
    static constexpr std::array<int, 8> KING_SHIELD_BONUS = {0, 10, 20, 6, 2, 0, 0, 0};

    // Evaluation methods
    int evaluateMaterial(const chess::Board &board) const;
    int evaluatePosition(const chess::Board &board, int gamePhaseScore) const;
    int evaluatePawnStructure(const chess::Board &board) const;
    int evaluateMobility(const chess::Board &board) const;
    int evaluateKingSafety(const chess::Board &board, int gamePhaseScore) const;
    int evaluateThreats(const chess::Board &board) const;
    int evaluatePieceCoordination(const chess::Board &board) const;
    bool isEndgame(const chess::Board &board) const;
    int calculateGamePhase(const chess::Board &board) const;

    // Helper methods
    int mirrorSquare(int square) const
    {
        return square ^ 56;
    }

    bool isOpenFile(const chess::Board &board, int file) const;
    bool isSemiOpenFile(const chess::Board &board, int file, chess::Color color) const;
    int countPawnShield(const chess::Board &board, int kingSq, chess::Color color) const;
    int getDistance(int sq1, int sq2) const;
    bool isPassed(const chess::Board &board, int sq, chess::Color color) const;
    bool isIsolated(const chess::Board &board, int sq, chess::Color color) const;
    bool isDoubled(const chess::Board &board, int sq, chess::Color color) const;
    bool isConnected(const chess::Board &board, int sq, chess::Color color) const;
};

#endif // EVALUATION_HPP