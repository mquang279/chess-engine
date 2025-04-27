#include "ChessEngine.hpp"
#include <algorithm>
#include <iomanip>
#include <ctime>

ChessEngine::ChessEngine() : rng(static_cast<unsigned int>(std::time(nullptr)))
{
    // Initialize random number generator with current time
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

    // Base case: if we reached the bottom of the search, evaluate the position
    if (depth == 0)
    {
        return quiesence(board, alpha, beta, nodes);
    }

    // Check for draw by repetition or fifty-move rule
    if (board.isRepetition() || board.isHalfMoveDraw())
    {
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

    // Simple move ordering: captures first
    // This significantly improves alpha-beta pruning
    for (int i = 0; i < moves.size(); ++i)
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

    return bestScore;
}

int ChessEngine::quiesence(chess::Board &board, int alpha, int beta,
    uint64_t &nodes) {
    nodes++;

    // Evaluate the current position
    int stand_pat = pestoEval.evaluate(board);

    // If the static evaluation is already worse than beta, prune this branch
    if (stand_pat >= beta) {
      return beta;
    }

    // Update alpha if the static evaluation is better
    if (stand_pat > alpha) {
      alpha = stand_pat;
    }

    // Generate all capture moves
    chess::Movelist moves;
    chess::movegen::legalmoves<chess::MoveGenType::CAPTURE>(moves, board);
    //MVV-LVA
    for (auto &move : moves) {
      auto victim = board.at(move.to());
      auto attacker = board.at(move.from());
      auto victim_type = chess::utils::typeOfPiece(victim);
      auto attacker_type = chess::utils::typeOfPiece(attacker);
      move.setScore(pestoEval.mg_value[pestoEval.pieceTypeToIndex(victim_type)] * 10 +
               (9 - pestoEval.mg_value[pestoEval.pieceTypeToIndex(attacker_type)]));
    }
    moves.sort();
    for (int i = 0; i < moves.size(); i++) {
      // Make the move
      board.makeMove(moves[i]);

      // Perform a recursive quiescence search
      int score = -quiesence(board, -beta, -alpha, nodes);

      // Undo the move
      board.unmakeMove(moves[i]);

      // Update alpha
      if (score > alpha) {
        // Beta cutoff
        if (score >= beta) {
          return beta;
        }
        alpha = score;
      }
    }

    return alpha;
}

int ChessEngine::evaluatePosition(const chess::Board &board)
{
    // Use our evaluation function
    return evaluation.evaluate(board);
}