import random
import chess
import sys
import os

# Add the parent directory to the Python path so we can import the chess_engine module
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    import chess_engine
except ImportError:
    print("Error: chess_engine module not found. Make sure it's built correctly.")
    sys.exit(1)

class ChessBot:
    def __init__(self):
        self.engine = chess_engine.ChessEngine()

    def choose_move(self, board):
        # Convert python-chess board to FEN and pass to C++ engine
        self.engine.set_position(board.fen())
        
        # Get best move from the C++ engine
        uci_move = self.engine.get_best_move()
        
        # Convert UCI string back to python-chess Move object
        return chess.Move.from_uci(uci_move)
    
    def evaluate(self, board):
        # This can be implemented if needed, but the engine already has evaluation
        return None