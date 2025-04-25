#ifndef CHESS_ENGINE_HPP
#define CHESS_ENGINE_HPP

#include "../chess.hpp"
#include <vector>
#include <map>
#include <random>

class ChessEngine
{
public:
    ChessEngine();
    ~ChessEngine() = default;

    // Get the best move for the current position
    chess::Move getBestMove(chess::Board &board);

private:
    // Alpha-beta pruning search algorithm
    int alphaBeta(chess::Board &board, int depth, int alpha, int beta, bool maximizingPlayer);

    // Static evaluation function
    int evaluatePosition(const chess::Board &board);

    // Piece values for material evaluation
    std::map<chess::PieceType, int> pieceValues;

    // Piece-square tables for positional evaluation
    std::map<chess::PieceType, std::array<int, 64>> pieceSquareTables;

    // Initialize evaluation tables
    void initializeEvaluationTables();

    // Random number generator for breaking ties or adding some randomness
    std::mt19937 rng;
};

#endif // CHESS_ENGINE_HPP