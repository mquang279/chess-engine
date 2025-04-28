#ifndef CHESS_ENGINE_HPP
#define CHESS_ENGINE_HPP

#include "../chess.hpp"
#include "Evaluation.hpp"
#include "OpeningMove.hpp"
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

    bool initializeOpeningBook();

    void setMaxBookMoves(int maxMoves);

    void enableOpeningBook(bool enable) { useOpeningBook = enable; }

    static constexpr int MAX_DEPTH = 6;
    static constexpr int TIME_LIMIT = 5;
    static constexpr int GOOD_CAPTURE_WEIGHT = 5000;

    // Constants for pruning techniques
    static constexpr int FUTILITY_MARGIN_BASE = 125;  // Base margin for futility pruning (centipawns)
    static constexpr int FUTILITY_MARGIN_MULTIPLIER = 100;  // Multiplier per depth
    static constexpr int LMP_BASE = 3;  // Base move count for late move pruning
    static constexpr int LMP_DEPTH_FACTOR = 3;  // Additional moves per depth level
    static constexpr int LMR_MIN_DEPTH = 3;  // Minimum depth for late move reduction
    static constexpr int LMR_MIN_MOVES = 3;  // Minimum moves before reduction

private:
    // Constants for searchMoves arrays
    static constexpr int NUM_PLIES = 64;
    static constexpr int NUM_MOVES = 256;
    OpeningMove openingBook;
    bool useOpeningBook = true;
    int moveCounter = 0;

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

    void printSearchInfo(const SearchStats &stats);

    bool hasNonPawnMaterial(const chess::Board &board) const;

    Evaluation evaluation;

    std::mt19937 rng;

    std::array<std::array<chess::Move, NUM_MOVES>, NUM_PLIES> searchMoves;
};

#endif // CHESS_ENGINE_HPP