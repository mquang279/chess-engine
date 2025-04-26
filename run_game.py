#!/usr/bin/env python3
"""
Chess Game Launcher
This script runs the chess game with the C++ engine integration.
"""
import os
import sys

# Add the project root to the Python path to find the chess_engine module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

# Change directory to project root to ensure asset paths are correct
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Import and run the UI
from ui.UI import UI

def main():
    try:
        # Try to import the chess_engine module to verify it's available
        import chess_engine
        print("C++ Chess Engine loaded successfully!")
    except ImportError as e:
        print(f"Warning: Could not import chess_engine module: {e}")
        print("Make sure you've built the C++ module with: ")
        print("g++ -O3 -Wall -shared -std=c++20 -fPIC $(python3 -m pybind11 --includes) " +
              "src/chess_engine_bindings.cpp src/engine/ChessEngine.cpp src/engine/PestoEvaluation.cpp " +
              "-o chess_engine$(python3-config --extension-suffix)")
        sys.exit(1)
    
    # Start the UI
    game_ui = UI()
    game_ui.run()

if __name__ == "__main__":
    main()