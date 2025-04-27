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

private:
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

    int negamax(chess::Board &board, int depth, int alpha, int beta, uint64_t &nodes);

    int evaluatePosition(const chess::Board &board);

    void printSearchInfo(const SearchStats &stats, const TTStats &tt_stats);

    Evaluation evaluation;

    std::mt19937 rng;
    
    TranspositionTable tt;
};

#endif // CHESS_ENGINE_HPP