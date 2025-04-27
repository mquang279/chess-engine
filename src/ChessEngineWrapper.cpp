#include "chess.hpp"
#include "engine/ChessEngine.hpp"
#include "engine/Evaluation.hpp"
#include <string>
#include <cstring>
#include <sstream>

// Platform-specific export macros
#ifdef _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif

// Simple wrapper class for the chess engine to be used from Python
class ChessEngineWrapper
{
private:
    ChessEngine engine;
    chess::Board board;
    Evaluation evaluator;

    // Cache for legal moves to avoid regenerating them frequently
    chess::Movelist moves_cache;
    bool moves_cache_valid = false;

    // Helper function to update legal moves cache
    void updateMovesCache()
    {
        if (!moves_cache_valid)
        {
            moves_cache.clear();
            chess::movegen::legalmoves(moves_cache, board);
            moves_cache_valid = true;
        }
    }

    // Helper function to find a move in the legal moves list
    chess::Move findMove(chess::Square fromSq, chess::Square toSq, chess::PieceType promotionType = chess::PieceType::NONE)
    {
        updateMovesCache();

        for (int i = 0; i < moves_cache.size(); ++i)
        {
            const auto &move = moves_cache[i];
            if (move.from() == fromSq && move.to() == toSq)
            {
                if (promotionType != chess::PieceType::NONE)
                {
                    if (move.typeOf() == chess::Move::PROMOTION && move.promotionType() == promotionType)
                    {
                        return move;
                    }
                }
                else if (move.typeOf() != chess::Move::PROMOTION)
                {
                    return move;
                }
            }
        }
        return chess::Move::NULL_MOVE;
    }

public:
    ChessEngineWrapper() : engine(), evaluator() {}

    // Set position from FEN string
    void setPosition(const std::string &fen)
    {
        board.setFen(fen);
        moves_cache_valid = false; // Invalidate cache
    }

    // Get the best move in UCI format (e.g., "e2e4")
    std::string getBestMove()
    {
        updateMovesCache();

        // If no legal moves, return empty string
        if (moves_cache.empty())
        {
            return "";
        }

        chess::Move move = engine.getBestMove(board);

        // Convert the move to a string
        std::stringstream ss;
        ss << move;
        return ss.str();
    }

    // Get the evaluation of the current position
    int getEvaluation()
    {
        return evaluator.evaluate(board);
    }

    // Make a move in UCI format
    bool makeMove(const std::string &moveStr)
    {
        if (moveStr.length() < 4)
            return false;

        // Extract source and destination squares
        chess::Square fromSq = chess::utils::extractSquare(moveStr.substr(0, 2));
        chess::Square toSq = chess::utils::extractSquare(moveStr.substr(2, 2));

        // Check for promotion
        chess::PieceType promotionType = chess::PieceType::NONE;
        if (moveStr.length() > 4)
        {
            char promotionChar = moveStr[4];
            switch (promotionChar)
            {
            case 'q':
                promotionType = chess::PieceType::QUEEN;
                break;
            case 'r':
                promotionType = chess::PieceType::ROOK;
                break;
            case 'b':
                promotionType = chess::PieceType::BISHOP;
                break;
            case 'n':
                promotionType = chess::PieceType::KNIGHT;
                break;
            default:
                break;
            }
        }

        // Find the move in legal moves
        chess::Move move = findMove(fromSq, toSq, promotionType);

        if (move == chess::Move::NULL_MOVE)
            return false;

        // Make the move
        board.makeMove(move);
        moves_cache_valid = false; // Invalidate cache after making a move
        return true;
    }

    // Get FEN string of current position
    std::string getFen()
    {
        return board.getFen();
    }

    // Check if the game is over
    bool isGameOver()
    {
        auto [reason, result] = board.isGameOver();
        return reason != chess::GameResultReason::NONE;
    }

    // Get all legal moves in the current position
    std::vector<std::string> getLegalMoves()
    {
        updateMovesCache();

        std::vector<std::string> result;
        result.reserve(moves_cache.size());

        for (int i = 0; i < moves_cache.size(); ++i)
        {
            std::stringstream ss;
            ss << moves_cache[i];
            result.push_back(ss.str());
        }

        return result;
    }

    // Reset the board to the starting position
    void resetBoard()
    {
        board.setFen(chess::STARTPOS);
        moves_cache_valid = false; // Invalidate cache
    }

    // Check if a specific move is legal
    bool isMoveLegal(const std::string &moveStr)
    {
        if (moveStr.length() < 4)
            return false;

        chess::Square fromSq = chess::utils::extractSquare(moveStr.substr(0, 2));
        chess::Square toSq = chess::utils::extractSquare(moveStr.substr(2, 2));

        // Check for promotion
        chess::PieceType promotionType = chess::PieceType::NONE;
        if (moveStr.length() > 4)
        {
            char promotionChar = moveStr[4];
            switch (promotionChar)
            {
            case 'q':
                promotionType = chess::PieceType::QUEEN;
                break;
            case 'r':
                promotionType = chess::PieceType::ROOK;
                break;
            case 'b':
                promotionType = chess::PieceType::BISHOP;
                break;
            case 'n':
                promotionType = chess::PieceType::KNIGHT;
                break;
            default:
                break;
            }
        }

        // Find the move in legal moves
        return findMove(fromSq, toSq, promotionType) != chess::Move::NULL_MOVE;
    }

    // Check if the current position is in check
    bool isInCheck()
    {
        return board.inCheck();
    }

    // Get the side to move (true for white, false for black)
    bool getSideToMove()
    {
        return board.sideToMove() == chess::Color::WHITE;
    }

    // Get the result of the game (0 = ongoing, 1 = white wins, -1 = black wins, 2 = draw)
    int getGameResult()
    {
        auto [reason, result] = board.isGameOver();

        if (reason == chess::GameResultReason::NONE)
        {
            return 0; // Game is ongoing
        }

        if (result == chess::GameResult::WIN)
        {
            return (board.sideToMove() == chess::Color::WHITE) ? -1 : 1;
        }
        else if (result == chess::GameResult::DRAW)
        {
            return 2;
        }

        return 0; // Default case
    }

    // Get a string description of the game result reason
    std::string getGameResultReason()
    {
        auto [reason, result] = board.isGameOver();

        switch (reason)
        {
        case chess::GameResultReason::CHECKMATE:
            return "Checkmate";
        case chess::GameResultReason::STALEMATE:
            return "Stalemate";
        case chess::GameResultReason::INSUFFICIENT_MATERIAL:
            return "Insufficient material";
        case chess::GameResultReason::FIFTY_MOVE_RULE:
            return "Fifty move rule";
        case chess::GameResultReason::THREEFOLD_REPETITION:
            return "Threefold repetition";
        default:
            return "Game in progress";
        }
    }
};

// Global wrapper instance
static ChessEngineWrapper *g_wrapper = nullptr;

// C interface for use with Python ctypes
extern "C"
{
    // Create a chess engine instance
    EXPORT_API void create_engine()
    {
        if (!g_wrapper)
        {
            g_wrapper = new ChessEngineWrapper();
        }
    }

    // Destroy the chess engine instance
    EXPORT_API void destroy_engine()
    {
        if (g_wrapper)
        {
            delete g_wrapper;
            g_wrapper = nullptr;
        }
    }

    // Set position from FEN string
    EXPORT_API void set_position(const char *fen)
    {
        if (g_wrapper && fen)
        {
            g_wrapper->setPosition(std::string(fen));
        }
    }

    // Get the best move as a string
    EXPORT_API void get_best_move(char *result, int max_length)
    {
        if (g_wrapper && result && max_length > 0)
        {
            std::string move = g_wrapper->getBestMove();
            strncpy(result, move.c_str(), max_length - 1);
            result[max_length - 1] = '\0';
        }
        else if (result && max_length > 0)
        {
            strncpy(result, "", max_length);
        }
    }

    // Make a move
    EXPORT_API bool make_move(const char *move)
    {
        return g_wrapper && move && g_wrapper->makeMove(std::string(move));
    }

    // Get FEN string of current position
    EXPORT_API void get_fen(char *result, int max_length)
    {
        if (g_wrapper && result && max_length > 0)
        {
            std::string fen = g_wrapper->getFen();
            strncpy(result, fen.c_str(), max_length - 1);
            result[max_length - 1] = '\0';
        }
        else if (result && max_length > 0)
        {
            strncpy(result, "", max_length);
        }
    }

    // Check if the game is over
    EXPORT_API bool is_game_over()
    {
        return g_wrapper && g_wrapper->isGameOver();
    }

    // Reset the board to the starting position
    EXPORT_API void reset_board()
    {
        if (g_wrapper)
        {
            g_wrapper->resetBoard();
        }
    }

    // Check if a specific move is legal
    EXPORT_API bool is_move_legal(const char *move)
    {
        return g_wrapper && move && g_wrapper->isMoveLegal(std::string(move));
    }

    // Check if the current position is in check
    EXPORT_API bool is_in_check()
    {
        return g_wrapper && g_wrapper->isInCheck();
    }

    // Get the side to move (true for white, false for black)
    EXPORT_API bool get_side_to_move()
    {
        return g_wrapper && g_wrapper->getSideToMove();
    }

    // Get the evaluation of the current position
    EXPORT_API int get_evaluation()
    {
        return g_wrapper ? g_wrapper->getEvaluation() : 0;
    }

    // Get the result of the game (0 = ongoing, 1 = white wins, -1 = black wins, 2 = draw)
    EXPORT_API int get_game_result()
    {
        return g_wrapper ? g_wrapper->getGameResult() : 0;
    }

    // Get a string description of the game result reason
    EXPORT_API void get_game_result_reason(char *result, int max_length)
    {
        if (g_wrapper && result && max_length > 0)
        {
            std::string reason = g_wrapper->getGameResultReason();
            strncpy(result, reason.c_str(), max_length - 1);
            result[max_length - 1] = '\0';
        }
        else if (result && max_length > 0)
        {
            strncpy(result, "", max_length);
        }
    }

    // Get all legal moves in the current position
    EXPORT_API void get_legal_moves(char *result, int max_length)
    {
        if (!g_wrapper || !result || max_length <= 0)
        {
            if (result && max_length > 0)
                strncpy(result, "", max_length);
            return;
        }

        std::vector<std::string> moves = g_wrapper->getLegalMoves();
        std::stringstream ss;

        for (size_t i = 0; i < moves.size(); ++i)
        {
            if (i > 0)
                ss << " ";
            ss << moves[i];
        }

        std::string movesStr = ss.str();
        strncpy(result, movesStr.c_str(), max_length - 1);
        result[max_length - 1] = '\0';
    }
}