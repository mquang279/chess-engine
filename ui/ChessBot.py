import random
import chess
import sys
import os
import traceback
import time  # Added time module for measuring engine performance

# Add the parent directory to the Python path so we can import modules
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Import our custom ChessEngineBridge instead of the pybind11 module
from ui.ChessEngineBridge import ChessEngineBridge

class ChessBot:
    def __init__(self):
        self.engine = None
        self.last_move_time = 0  # Store the time taken for the last move
        try:
            # Use the bridge instead of direct pybind11 bindings
            self.engine = ChessEngineBridge()
        except ImportError as e:
            print(f"Error: Could not initialize chess engine: {e}")
            print("Make sure you've built the chess_engine_wrapper library correctly.")
            print("Try running 'mingw32-make clean' followed by 'mingw32-make' in the project root.")
        except Exception as e:
            print(f"Unexpected error initializing chess engine: {e}")
            traceback.print_exc()

    def choose_move(self, board):
        if not self.engine:
            # Fallback to a simple random move selection if the engine isn't available
            print("Warning: Using fallback random move selection (engine not available)")
            legal_moves = list(board.legal_moves)
            return random.choice(legal_moves) if legal_moves else None
            
        # Convert python-chess board to FEN and pass to the C++ engine
        try:
            self.engine.set_position(board.fen())
            
            # Measure the time taken for the engine to find the best move
            start_time = time.time()
            best_move = self.engine.get_best_move()
            end_time = time.time()
            
            # Calculate and store the time taken
            self.last_move_time = end_time - start_time
            print(f"Engine calculation time: {self.last_move_time:.4f} seconds")
            
            if best_move:
                return best_move
                
            # If engine returns None, fall back to random move
            legal_moves = list(board.legal_moves)
            return random.choice(legal_moves) if legal_moves else None
        except Exception as e:
            print(f"Error getting move from engine: {e}")
            # Fall back to random move
            legal_moves = list(board.legal_moves)
            return random.choice(legal_moves) if legal_moves else None
    
    def evaluate(self, board):
        # Use our engine's evaluation function
        if not self.engine:
            return None
            
        try:
            # Set the position and get evaluation
            self.engine.set_position(board.fen())
            return self.engine.get_evaluation()
        except Exception as e:
            print(f"Error getting evaluation from engine: {e}")
            return None
            
    def get_last_move_time(self):
        """Return the time taken for the last engine move calculation"""
        return self.last_move_time