#include "ChessEngine.hpp"
#include <algorithm>
#include <iomanip>
#include <ctime>

ChessEngine::ChessEngine()
    : rng(static_cast<unsigned int>(std::time(nullptr))),
      tt(64)
{}

void ChessEngine::printSearchInfo(const SearchStats &stats, const TTStats &tt_stats) 
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

    std::cout << "Transposition Table Stats: "
                        << "Size: " << tt_stats.size << "  "
                        << "Capacity: " << tt_stats.capacity << "  "
                        << "Usage: " << std::fixed << std::setprecision(2) << tt_stats.usage << "%  "
                        << "Hit Rate: " << tt_stats.hit_rate << "%  "
                        << std::endl;
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
    std::chrono::seconds timeLimit(TIME_LIMIT);

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
            int score = -negamax(board, depth - 1, -beta, -alpha, nodesSearched);
            stats.nodes += nodesSearched;

            board.unmakeMove(move);

            if (score > bestScore ||
                (score == bestScore && std::uniform_int_distribution<>(0, 100)(rng) < 10))
            { // Small chance to pick equal moves for variety
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

                auto tt_stats = tt.get_stats();

                printSearchInfo(stats, tt_stats);

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

        auto tt_stats = tt.get_stats();

        // Print info for this depth
        printSearchInfo(stats, tt_stats);

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

    uint64_t hash_key = board.hash();
    auto [hit, tt_score] = tt.lookup(hash_key, depth, alpha, beta);
    if (hit) {
        return tt_score;
    }
    // Base case: reached leaf node or terminal position
    if (depth == 0)
    {
        int score = evaluatePosition(board);
        tt.store(hash_key, score, TTFlag::EXACT_SCORE, depth);
        return score;
    }

    // Check for draw by repetition or fifty-move rule
    if (board.isRepetition() || board.isHalfMoveDraw())
    {
        tt.store(hash_key, 0, TTFlag::EXACT_SCORE, depth);
        return 0; // Draw evaluation is 0
    }

    // Generate legal moves
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // Check for checkmate or stalemate
    if (moves.empty())
    {
        if (board.inCheck())
        {
            // Checkmate: return worst possible score, adjusted for depth
            // We add depth to prefer shorter paths to mate
            return -30000 + depth;
        }
        else
        {
            // Stalemate: return draw score
            return 0;
        }
    }

    int bestScore = std::numeric_limits<int>::min();
    int original_alpha = alpha;
    for (int i = 0; i < moves.size(); i++)
    {
        chess::Move &move = moves[i];
        int score = 0;

        if (board.isCapture(move))
        {
            score = 10000;
        }

        move.setScore(score);
    }

    // Sort the moves by score in descending order
    moves.sort();

    int bestScore = -30001; // Worst possible score

    // Try each move and recursively evaluate the resulting position
    for (const auto &move : moves)
    {
        // Make the move on the board
        board.makeMove(move);

        // Recursive call with negated bounds (negamax)
        int score = -negamax(board, depth - 1, -beta, -alpha, nodes);

        // Unmake the move to restore the board
        board.unmakeMove(move);

        // Update best score
        bestScore = std::max(bestScore, score);

        // Alpha-beta pruning
        alpha = std::max(alpha, score);
        if (alpha >= beta)
        {
            // Beta cutoff - the opponent won't allow this position
            break;
        }
    }

    TTFlag flag;
    if (bestScore <= original_alpha) {
        flag = TTFlag::UPPER_BOUND;
    } else if (bestScore >= beta) {
        flag = TTFlag::LOWER_BOUND;
    } else {
        flag = TTFlag::EXACT_SCORE;
    }

    tt.store(hash_key, bestScore, flag, depth);
    return bestScore;
}

int ChessEngine::evaluatePosition(const chess::Board &board)
{
    // Use our evaluation function
    return evaluation.evaluate(board);
}