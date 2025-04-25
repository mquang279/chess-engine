#include "ChessEngine.hpp"
#include <algorithm>
#include <ctime>
#include <limits>

ChessEngine::ChessEngine() : rng(static_cast<unsigned int>(std::time(nullptr)))
{
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

    chess::Move bestMove = chess::Move::NULL_MOVE;
    int bestScore = std::numeric_limits<int>::min();

    // Alpha-beta parameters
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    // Try each move and evaluate the resulting position
    for (int i = 0; i < moves.size(); i++)
    {
        chess::Move move = moves[i];
        board.makeMove(move);

        // Use alpha-beta pruning with depth 3
        int score = alphaBeta(board, 3, alpha, beta, false);

        board.unmakeMove(move);

        // Update best move if this move is better
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
            alpha = std::max(alpha, bestScore);
        }
    }

    return bestMove;
}

int ChessEngine::alphaBeta(chess::Board &board, int depth, int alpha, int beta, bool maximizingPlayer)
{
    // Base case: reached the maximum depth or terminal position
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
            return maximizingPlayer ? -10000 : 10000; // Large negative/positive value for checkmate
        }
        else
        {
            return 0; // Stalemate is a draw (0)
        }
    }

    if (maximizingPlayer)
    {
        int maxEval = std::numeric_limits<int>::min();

        for (int i = 0; i < moves.size(); i++)
        {
            board.makeMove(moves[i]);
            int eval = alphaBeta(board, depth - 1, alpha, beta, false);
            board.unmakeMove(moves[i]);

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            if (beta <= alpha)
            {
                break; // Beta cutoff
            }
        }

        return maxEval;
    }
    else
    {
        int minEval = std::numeric_limits<int>::max();

        for (int i = 0; i < moves.size(); i++)
        {
            board.makeMove(moves[i]);
            int eval = alphaBeta(board, depth - 1, alpha, beta, true);
            board.unmakeMove(moves[i]);

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            if (beta <= alpha)
            {
                break; // Alpha cutoff
            }
        }

        return minEval;
    }
}