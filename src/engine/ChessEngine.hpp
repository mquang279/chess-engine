#ifndef CHESS_ENGINE_HPP
#define CHESS_ENGINE_HPP

#include "../chess.hpp"
#include "Evaluation.hpp"
#include "OpeningMove.hpp"
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

    bool initializeOpeningBook();

    void setMaxBookMoves(int maxMoves);

    void enableOpeningBook(bool enable) { useOpeningBook = enable; }

    static constexpr int MAX_DEPTH = 7;
    static constexpr int TIME_LIMIT = 10;
    static constexpr int GOOD_CAPTURE_WEIGHT = 5000;
    static constexpr int INF = 32000;
    // Define a mate score that's well below the infinity limit but leaves room for ply adjustment
    static constexpr int MATE_VALUE = 30000;
    static constexpr int CHECKMATE_SCORE = MATE_VALUE;
    static constexpr int DRAW_SCORE = 0;

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

    int negamax(chess::Board &board, int depth, int ply, int alpha, int beta,
                uint64_t &nodes);

    int quiesence(chess::Board &board, int alpha, int beta, uint64_t &nodes, int ply = 0);

    void orderMoves(chess::Board &board, chess::Movelist &moves);

    void scoreMoves(const chess::Board &board, chess::Move &move);

    int evaluatePosition(const chess::Board &board);

    void printSearchInfo(const SearchStats &stats);

    // Null move pruning helper functions
    bool hasNonPawnMaterial(const chess::Board &board) const;

    bool isEndGame(const chess::Board &board) const;

    bool isPossibleZugzwang(const chess::Board &board) const;

    bool verifyNullMovePrune(chess::Board &board, int depth, int beta, uint64_t &nodes);

    Evaluation evaluation;

    std::mt19937 rng;

    std::array<std::array<chess::Move, NUM_MOVES>, NUM_PLIES> searchMoves;

    TranspositionTable tt;
};

#endif // CHESS_ENGINE_HPP