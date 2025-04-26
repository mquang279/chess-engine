#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "chess.hpp"
#include "engine/ChessEngine.hpp"
#include <string>

namespace py = pybind11;

// Simple wrapper class for the chess engine to be used from Python
class ChessEngineWrapper {
private:
    ChessEngine engine;
    chess::Board board;

public:
    ChessEngineWrapper() : engine() {}
    
    // Set position from FEN string
    void setPosition(const std::string& fen) {
        board.setFen(fen);
    }
    
    // Get the best move in UCI format (e.g., "e2e4")
    std::string getBestMove() {
        chess::Move move = engine.getBestMove(board);
        
        // Convert the move to a string
        std::stringstream ss;
        ss << move;
        return ss.str();
    }
    
    // Make a move in UCI format
    bool makeMove(const std::string& moveStr) {
        if (moveStr.length() < 4) return false;
        
        // Extract source and destination squares
        chess::Square fromSq = chess::utils::extractSquare(moveStr.substr(0, 2));
        chess::Square toSq = chess::utils::extractSquare(moveStr.substr(2, 2));
        
        // Create and validate the move
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        
        // Try to find the move in the list of legal moves
        chess::Move move;
        bool foundMove = false;
        
        for (int i = 0; i < moves.size(); ++i) {
            if (moves[i].from() == fromSq && moves[i].to() == toSq) {
                // Check for promotion
                if (moveStr.length() > 4) {
                    char promotionChar = moveStr[4];
                    chess::PieceType promotionType;
                    
                    switch(promotionChar) {
                        case 'q': promotionType = chess::PieceType::QUEEN; break;
                        case 'r': promotionType = chess::PieceType::ROOK; break;
                        case 'b': promotionType = chess::PieceType::BISHOP; break;
                        case 'n': promotionType = chess::PieceType::KNIGHT; break;
                        default: continue; // Skip if invalid promotion piece
                    }
                    
                    if (moves[i].promotionType() == promotionType) {
                        move = moves[i];
                        foundMove = true;
                        break;
                    }
                } else if (moves[i].typeOf() != chess::Move::PROMOTION) {
                    move = moves[i];
                    foundMove = true;
                    break;
                }
            }
        }
        
        if (!foundMove) return false;
        
        // Make the move
        board.makeMove(move);
        return true;
    }
    
    // Get FEN string of current position
    std::string getFen() {
        return board.getFen();
    }
    
    // Check if the game is over
    bool isGameOver() {
        auto [reason, result] = board.isGameOver();
        return reason != chess::GameResultReason::NONE;
    }
    
    // Get all legal moves in the current position
    std::vector<std::string> getLegalMoves() {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        
        std::vector<std::string> result;
        for (int i = 0; i < moves.size(); ++i) {
            std::stringstream ss;
            ss << moves[i];
            result.push_back(ss.str());
        }
        
        return result;
    }
    
    // Reset the board to the starting position
    void resetBoard() {
        board.setFen(chess::STARTPOS);
    }
    
    // Check if a specific move is legal
    bool isMoveLegal(const std::string& moveStr) {
        if (moveStr.length() < 4) return false;
        
        chess::Square fromSq = chess::utils::extractSquare(moveStr.substr(0, 2));
        chess::Square toSq = chess::utils::extractSquare(moveStr.substr(2, 2));
        
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        
        for (int i = 0; i < moves.size(); ++i) {
            if (moves[i].from() == fromSq && moves[i].to() == toSq) {
                // Check for promotion
                if (moveStr.length() > 4 && moves[i].typeOf() == chess::Move::PROMOTION) {
                    char promotionChar = moveStr[4];
                    chess::PieceType promotionType;
                    
                    switch(promotionChar) {
                        case 'q': promotionType = chess::PieceType::QUEEN; break;
                        case 'r': promotionType = chess::PieceType::ROOK; break;
                        case 'b': promotionType = chess::PieceType::BISHOP; break;
                        case 'n': promotionType = chess::PieceType::KNIGHT; break;
                        default: continue;
                    }
                    
                    if (moves[i].promotionType() == promotionType) {
                        return true;
                    }
                } else if (moves[i].typeOf() != chess::Move::PROMOTION) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    // Check if the current position is in check
    bool isInCheck() {
        return board.inCheck();
    }
    
    // Get the side to move (true for white, false for black)
    bool getSideToMove() {
        return board.sideToMove() == chess::Color::WHITE;
    }
};

PYBIND11_MODULE(chess_engine, m) {
    m.doc() = "Python bindings for the C++ chess engine";
    
    py::class_<ChessEngineWrapper>(m, "ChessEngine")
        .def(py::init<>())
        .def("set_position", &ChessEngineWrapper::setPosition)
        .def("get_best_move", &ChessEngineWrapper::getBestMove)
        .def("make_move", &ChessEngineWrapper::makeMove)
        .def("get_fen", &ChessEngineWrapper::getFen)
        .def("is_game_over", &ChessEngineWrapper::isGameOver)
        .def("get_legal_moves", &ChessEngineWrapper::getLegalMoves)
        .def("reset_board", &ChessEngineWrapper::resetBoard)
        .def("is_move_legal", &ChessEngineWrapper::isMoveLegal)
        .def("is_in_check", &ChessEngineWrapper::isInCheck)
        .def("get_side_to_move", &ChessEngineWrapper::getSideToMove);
}