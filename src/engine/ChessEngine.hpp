#ifndef CHESS_ENGINE_HPP
#define CHESS_ENGINE_HPP

#include "../chess.hpp"
#include "Evaluation.hpp"
#include "transposition_table.hpp"
#include <vector>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>

class ChessEngine
{
public:
    ChessEngine();
    ~ChessEngine() = default;

    chess::Move getBestMove(chess::Board &board);

    static constexpr int MAX_DEPTH = 5;
    static constexpr int TIME_LIMIT = 5;
    static constexpr int GOOD_CAPTURE_WEIGHT = 5000;

private:
    // Constants for searchMoves arrays
    static constexpr int NUM_PLIES = 64;
    static constexpr int NUM_MOVES = 256;

    struct SearchStats
    {
        int depth = 0;
        int score = 0;
        uint64_t nodes = 0;
        std::chrono::milliseconds duration;
        chess::Move bestMove = chess::Move::NULL_MOVE;

        void reset()
        {
            nodes = 0;
            score = 0;
            bestMove = chess::Move::NULL_MOVE;
        }
    };

    int negamax(chess::Board &board, int depth, int alpha, int beta,
                uint64_t &nodes);

    int quiesence(chess::Board &board, int alpha, int beta, uint64_t &nodes);

    void orderMoves(chess::Board &board, chess::Movelist &moves);

    void scoreMoves(const chess::Board &board, chess::Move &move);

    int evaluatePosition(const chess::Board &board);

    void printSearchInfo(const SearchStats &stats, const TTStats &tt_stats);

    Evaluation evaluation;

    std::mt19937 rng;

    TranspositionTable tt;

    std::array<std::array<chess::Move, NUM_MOVES>, NUM_PLIES> searchMoves;
};

#endif // CHESS_ENGINE_HPP