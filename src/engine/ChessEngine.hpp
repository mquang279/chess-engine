#ifndef CHESS_ENGINE_HPP
#define CHESS_ENGINE_HPP

#include "../chess.hpp"
#include "PestoEvaluation.hpp"
#include "transposition_table.hpp"
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <iostream>

class ChessEngine
{
public:
    ChessEngine();
    ~ChessEngine() = default;

    // Get the best move for the current position
    chess::Move getBestMove(chess::Board &board);

    // Maximum search depth
    const int MAX_DEPTH = 6;
    const int TIME_LIMIT = 10;

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

    void printSearchInfo(const SearchStats &stats);

    PestoEvaluation pestoEval;

    // Random number generator for breaking ties or adding some randomness
    std::mt19937 rng;
    
    TranspositionTable tt;
};

#endif // CHESS_ENGINE_HPP