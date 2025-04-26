#!/usr/bin/env python3
"""
Chess Game Launcher
This script runs the chess game with the C++ engine integration.
"""
import os
import sys
import platform
import subprocess

# Add the project root to the Python path to find modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

# Change directory to project root to ensure asset paths are correct
os.chdir(os.path.dirname(os.path.abspath(__file__)))

def check_engine_library():
    """Check if the chess engine library exists"""
    base_dir = os.path.dirname(os.path.abspath(__file__))
    if platform.system() == "Windows":
        lib_path = os.path.join(base_dir, "chess_engine_wrapper.dll")
    elif platform.system() == "Darwin":  # macOS
        lib_path = os.path.join(base_dir, "chess_engine_wrapper.dylib")
    else:  # Linux and others
        lib_path = os.path.join(base_dir, "chess_engine_wrapper.so")
    
    return os.path.exists(lib_path)

def main():
    try:
        # Import UI to check if it exists before proceeding
        from ui.UI import UI
        
        # Check if the chess engine library exists
        if not check_engine_library():
            print("Chess engine library not found.")
            print("Please build it first using 'make' or 'mingw32-make' (on Windows)")
            print("Attempting to build the library for you...")
            
            # Try to build using make
            try:
                if platform.system() == "Windows":
                    # On Windows, try both make and mingw32-make
                    try:
                        subprocess.run(["mingw32-make"], check=True)
                    except (FileNotFoundError, subprocess.SubprocessError):
                        subprocess.run(["make"], check=True)
                else:
                    # On Unix-like systems, use make
                    subprocess.run(["make"], check=True)
                
                print("Chess engine compiled successfully!")
            except Exception as e:
                print(f"Failed to compile chess engine: {e}")
                print("\nPlease compile manually using one of these commands:")
                print("- make")
                print("- mingw32-make (if using MinGW on Windows)")
                print("\nThe game will run with limited functionality.")
        
        # Start the UI
        print("Starting Chess UI...")
        game_ui = UI()
        game_ui.run()
    except ImportError as e:
        print(f"Error: Could not import required module: {e}")
        print("Make sure all Python dependencies are installed and the project structure is correct.")
    except Exception as e:
        print(f"Error running the game: {e}")

if __name__ == "__main__":
    main()