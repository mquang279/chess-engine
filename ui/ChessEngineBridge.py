import ctypes
import os
import sys
import platform
import chess
import traceback
from ctypes import wintypes

class ChessEngineBridge:
    """
    Bridge class to connect Python with the C++ ChessEngineWrapper
    using ctypes instead of pybind11
    """
    def __init__(self):
        # Determine the correct library extension based on platform
        if platform.system() == "Windows":
            lib_ext = ".dll"
        elif platform.system() == "Darwin":  # macOS
            lib_ext = ".dylib"
        else:  # Linux and others
            lib_ext = ".so"
        
        # Get the project root directory
        proj_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        
        # Path to the compiled library
        lib_path = os.path.join(proj_root, f"chess_engine_wrapper{lib_ext}")
        
        # Detailed diagnostic info for troubleshooting
        print(f"Attempting to load chess engine from: {lib_path}")
        print(f"File exists: {os.path.exists(lib_path)}")
        if os.path.exists(lib_path):
            print(f"File size: {os.path.getsize(lib_path)} bytes")
        
        try:
            # On Windows, use LoadLibrary with specific error handling
            if platform.system() == "Windows":
                # Add the directory to PATH temporarily to help find dependencies
                current_path = os.environ.get('PATH', '')
                os.environ['PATH'] = f"{proj_root};{current_path}"
                
                # Use direct Windows API calls for better error handling
                try:
                    # First try kernel32.LoadLibraryEx with better error handling
                    kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
                    kernel32.LoadLibraryExW.argtypes = [wintypes.LPCWSTR, wintypes.HANDLE, wintypes.DWORD]
                    kernel32.LoadLibraryExW.restype = wintypes.HMODULE
                    
                    # Load the library with LOAD_WITH_ALTERED_SEARCH_PATH to use the directory of the DLL for dependencies
                    LOAD_WITH_ALTERED_SEARCH_PATH = 0x00000008
                    h_module = kernel32.LoadLibraryExW(lib_path, None, LOAD_WITH_ALTERED_SEARCH_PATH)
                    
                    if not h_module:
                        error_code = ctypes.get_last_error()
                        raise ctypes.WinError(error_code)
                    
                    # Now use the handle with WinDLL
                    self.lib = ctypes.WinDLL(None, handle=h_module)
                except Exception as win_ex:
                    print(f"Advanced loading failed: {win_ex}")
                    print("Falling back to standard loading method...")
                    # Fall back to standard loading
                    self.lib = ctypes.WinDLL(lib_path)
            else:
                self.lib = ctypes.CDLL(lib_path)
                
            print(f"Successfully loaded chess engine from: {lib_path}")
        except Exception as e:
            # Very detailed error message with troubleshooting info
            error_msg = f"Failed to load chess engine library from {lib_path}: {e}\n"
            
            if platform.system() == "Windows":
                error_msg += "\nPossible causes and solutions:"
                error_msg += "\n1. Missing Visual C++ Runtime - Install the latest VC++ Redistributable"
                error_msg += "\n2. DLL was compiled with incompatible compiler - Try rebuilding"
                error_msg += "\n3. Missing dependencies - Check with Dependency Walker tool"
                
                # Try to get the Windows error code
                error_code = ctypes.get_last_error()
                if error_code:
                    error_msg += f"\nWindows error code: {error_code}"
                    try:
                        error_msg += f" ({ctypes.FormatError(error_code)})"
                    except:
                        pass
                        
            traceback.print_exc()
            raise ImportError(error_msg)
            
        # Define function signatures
        
        # void create_engine()
        self.lib.create_engine.argtypes = []
        self.lib.create_engine.restype = None
        
        # void destroy_engine()
        self.lib.destroy_engine.argtypes = []
        self.lib.destroy_engine.restype = None
        
        # void set_position(const char* fen)
        self.lib.set_position.argtypes = [ctypes.c_char_p]
        self.lib.set_position.restype = None
        
        # void get_best_move(char* result, int max_length)
        self.lib.get_best_move.argtypes = [ctypes.c_char_p, ctypes.c_int]
        self.lib.get_best_move.restype = None
        
        # bool make_move(const char* move)
        self.lib.make_move.argtypes = [ctypes.c_char_p]
        self.lib.make_move.restype = ctypes.c_bool
        
        # void get_fen(char* result, int max_length)
        self.lib.get_fen.argtypes = [ctypes.c_char_p, ctypes.c_int]
        self.lib.get_fen.restype = None
        
        # bool is_game_over()
        self.lib.is_game_over.argtypes = []
        self.lib.is_game_over.restype = ctypes.c_bool
        
        # void reset_board()
        self.lib.reset_board.argtypes = []
        self.lib.reset_board.restype = None
        
        # bool is_move_legal(const char* move)
        self.lib.is_move_legal.argtypes = [ctypes.c_char_p]
        self.lib.is_move_legal.restype = ctypes.c_bool
        
        # bool is_in_check()
        self.lib.is_in_check.argtypes = []
        self.lib.is_in_check.restype = ctypes.c_bool
        
        # bool get_side_to_move()
        self.lib.get_side_to_move.argtypes = []
        self.lib.get_side_to_move.restype = ctypes.c_bool
        
        # int get_evaluation()
        self.lib.get_evaluation.argtypes = []
        self.lib.get_evaluation.restype = ctypes.c_int
        
        # Initialize the engine
        self.lib.create_engine()
        
    def __del__(self):
        """Clean up when the object is deleted"""
        if hasattr(self, 'lib'):
            self.lib.destroy_engine()
    
    def set_position(self, fen):
        """Set the board position using FEN notation"""
        self.lib.set_position(fen.encode('utf-8'))
    
    def get_best_move(self):
        """Get the best move for the current position as a python-chess Move object"""
        # Create a buffer for the result (UCI moves are typically 5 chars max)
        buffer_size = 10
        result_buffer = ctypes.create_string_buffer(buffer_size)
        
        # Call the C function
        self.lib.get_best_move(result_buffer, buffer_size)
        
        # Convert the result to a string and then to a python-chess Move
        uci_move = result_buffer.value.decode('utf-8')
        
        if not uci_move:
            return None
            
        try:
            return chess.Move.from_uci(uci_move)
        except ValueError:
            return None
    
    def make_move(self, move):
        """Make a move on the board (takes python-chess Move object)"""
        if isinstance(move, chess.Move):
            move_str = move.uci()
        else:
            move_str = str(move)
            
        return self.lib.make_move(move_str.encode('utf-8'))
    
    def get_fen(self):
        """Get the current position in FEN notation"""
        buffer_size = 100  # FEN strings can be long
        result_buffer = ctypes.create_string_buffer(buffer_size)
        
        self.lib.get_fen(result_buffer, buffer_size)
        return result_buffer.value.decode('utf-8')
    
    def is_game_over(self):
        """Check if the game is over"""
        return self.lib.is_game_over()
    
    def reset_board(self):
        """Reset the board to the starting position"""
        self.lib.reset_board()
    
    def is_move_legal(self, move):
        """Check if a move is legal (takes python-chess Move object)"""
        if isinstance(move, chess.Move):
            move_str = move.uci()
        else:
            move_str = str(move)
            
        return self.lib.is_move_legal(move_str.encode('utf-8'))
    
    def is_in_check(self):
        """Check if the current player is in check"""
        return self.lib.is_in_check()
    
    def get_side_to_move(self):
        """Get the side to move (True for white, False for black)"""
        return self.lib.get_side_to_move()
        
    def get_evaluation(self):
        """Get the evaluation of the current position"""
        return self.lib.get_evaluation()