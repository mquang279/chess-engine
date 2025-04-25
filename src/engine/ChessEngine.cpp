#include "ChessEngine.hpp"
#include <algorithm>
#include <ctime>
#include <limits>
#include <iomanip>
#include <sstream>

ChessEngine::ChessEngine() : rng(static_cast<unsigned int>(std::time(nullptr)))
{
}

void ChessEngine::printSearchInfo(const SearchStats &stats)
{
    float timeInSec = stats.duration.count() / 1000.0f;

    // Calculate nodes per second
    uint64_t nps = (timeInSec > 0) ? static_cast<uint64_t>(stats.nodes / timeInSec) : 0;

    std::cout << "Depth: " << stats.depth
              << ", Evaluation: " << stats.score
              << ", Nodes: " << stats.nodes
              << ", Time: " << stats.duration.count() << "ms"
              << ", NPS: " << nps;

    if (stats.bestMove != chess::Move::NULL_MOVE)
    {
        std::cout << ", Best move: " << stats.bestMove << std::endl;
    }
}

chess::Move ChessEngine::getBestMove(chess::Board &board)
{
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    if (moves.empty())
    {
        return chess::Move::NULL_MOVE;
    }

    // If only one move is available, return it immediately
    if (moves.size() == 1)
    {
        return moves[0];
    }

    // For iterative deepening
    chess::Move bestMove = chess::Move::NULL_MOVE;
    chess::Move previousBestMove = chess::Move::NULL_MOVE;
    SearchStats stats;

    // Start timing the entire search
    auto startTime = std::chrono::high_resolution_clock::now();
    std::chrono::seconds timeLimit(TIME_LIMIT); // Use the TIME_LIMIT constant from the header

    // Iterative deepening - from depth 1 to MAX_DEPTH
    for (int depth = 1; depth <= MAX_DEPTH; depth++)
    {
        stats.reset();
        stats.depth = depth;

        // For measuring time at each depth
        auto depthStartTime = std::chrono::high_resolution_clock::now();

        // Alpha-beta parameters
        int alpha = std::numeric_limits<int>::min();
        int beta = std::numeric_limits<int>::max();
        int bestScore = std::numeric_limits<int>::min();

        // Try each move and evaluate the resulting position
        for (int i = 0; i < moves.size(); i++)
        {
            chess::Move move = moves[i];
            board.makeMove(move);

            uint64_t nodesSearched = 0;
            int score = negamax(board, depth - 1, alpha, beta, nodesSearched);
            stats.nodes += nodesSearched;

            board.unmakeMove(move);

            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
                alpha = std::max(alpha, bestScore);
            }

            // Check if we've exceeded the time limit
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
            if (elapsedTime >= timeLimit)
            {
                std::cout << "Search time limit exceeded (" << timeLimit.count() << " seconds). Stopping search." << std::endl;

                // Print partial info for this depth
                auto depthEndTime = std::chrono::high_resolution_clock::now();
                stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(depthEndTime - depthStartTime);
                stats.score = bestScore;
                stats.bestMove = bestMove;
                printSearchInfo(stats);

                // Print total search time
                auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime);
                std::cout << "Total search time: " << totalDuration.count() << " ms" << std::endl;

                return bestMove != chess::Move::NULL_MOVE ? bestMove : previousBestMove;
            }
        }

        // Update stats for this depth
        auto depthEndTime = std::chrono::high_resolution_clock::now();
        stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(depthEndTime - depthStartTime);
        stats.score = bestScore;
        stats.bestMove = bestMove;

        // Print info for this depth
        printSearchInfo(stats);

        // Save the best move from this depth as our current best
        previousBestMove = bestMove;

        // Check if we've exceeded the time limit after completing a depth
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        if (elapsedTime >= timeLimit)
        {
            std::cout << "Search time limit exceeded (" << timeLimit.count() << " seconds). Stopping search." << std::endl;
            break;
        }
    }

    // Print total search time
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "Total search time: " << totalDuration.count() << " ms" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
    return bestMove;
}

int ChessEngine::negamax(chess::Board &board, int depth, int alpha, int beta, uint64_t &nodes)
{
    nodes++;

    // Base case: reached leaf node or terminal position
    if (depth == 0 || board.isGameOver().first != chess::GameResultReason::NONE)
    {
        return evaluatePosition(board);
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // No legal moves (checkmate or stalemate)
    if (moves.empty())
    {
        // If in check, it's a checkmate
        if (board.inCheck())
        {
            return -10000 + depth; // Prefer shorter mates: lower depth = closer mate
        }
        else
        {
            return 0; // Stalemate is a draw (0)
        }
    }

    int bestScore = std::numeric_limits<int>::min();

    for (int i = 0; i < moves.size(); i++)
    {
        board.makeMove(moves[i]);
        // Negamax recursion with negated alpha/beta window and negated return value
        int score = -negamax(board, depth - 1, -beta, -alpha, nodes);
        board.unmakeMove(moves[i]);

        // Update best score
        bestScore = std::max(bestScore, score);

        // Update alpha for pruning
        alpha = std::max(alpha, score);
        if (alpha >= beta)
        {
            break; // Beta cutoff
        }
    }

    return bestScore;
}

int ChessEngine::evaluatePosition(const chess::Board &board)
{
    // Use Pesto's evaluation function
    return pestoEval.evaluate(board);
}