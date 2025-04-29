import time
import chess
import sys
import os

# Add parent directory to path for module imports
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from ui.ChessBot import ChessBot
from ui.GameMode import GameMode


class ChessGame:

    def __init__(self, mode=GameMode.HUMAN_VS_HUMAN, human_side=chess.WHITE):
        self.board = chess.Board()
        self.selected_square = None
        self.last_move = None
        self.mode = mode
        self.human_side = human_side
        # Callback functions for move sound effects
        self.on_move_callback = None
        self.on_capture_callback = None
        self.on_check_callback = None
        
        # Track engine calculation time
        self.last_engine_calc_time = 0

        # Initialize bots based on mode and human_side
        if mode == GameMode.BOT_VS_BOT:
            self.white_bot = ChessBot()
            self.black_bot = ChessBot()
        elif mode == GameMode.HUMAN_VS_BOT:
            if human_side == chess.WHITE:
                self.white_bot = None
                self.black_bot = ChessBot()
            else:  # human is black
                self.white_bot = ChessBot()
                self.black_bot = None
        else:  # HUMAN_VS_HUMAN
            self.white_bot = None
            self.black_bot = None

        self.game_over = False
        self.waiting_for_bot = False
        self.bot_move_time = 0.5
        self.last_bot_move_time = 0
    
    def register_callbacks(self, move_cb=None, capture_cb=None, check_cb=None):
        """Register callback functions for move events"""
        self.on_move_callback = move_cb
        self.on_capture_callback = capture_cb
        self.on_check_callback = check_cb

    def is_valid_move(self, from_square, to_square):
        move = chess.Move(from_square, to_square)

        if (self.board.piece_at(from_square) and
                self.board.piece_at(from_square).piece_type == chess.PAWN and
                ((to_square // 8 == 7 and self.board.turn == chess.WHITE) or
                 (to_square // 8 == 0 and self.board.turn == chess.BLACK))):
            move = chess.Move(from_square, to_square, promotion=chess.QUEEN)

        return move in self.board.legal_moves

    def make_move(self, from_square, to_square):
        move = chess.Move(from_square, to_square)

        if (self.board.piece_at(from_square) and
                self.board.piece_at(from_square).piece_type == chess.PAWN and
                ((to_square // 8 == 7 and self.board.turn == chess.WHITE) or
                 (to_square // 8 == 0 and self.board.turn == chess.BLACK))):
            move = chess.Move(from_square, to_square, promotion=chess.QUEEN)

        if move in self.board.legal_moves:
            # Check if this is a capture before making the move
            is_capture = self.board.piece_at(to_square) is not None
            
            # Make the move
            self.board.push(move)
            self.last_move = move
            
            # Check game ending conditions
            if self.board.is_checkmate() or self.board.is_stalemate() or \
                    self.board.is_insufficient_material() or self.board.is_seventyfive_moves() or \
                    self.board.is_fivefold_repetition():
                self.game_over = True

            return True
        return False

    def make_bot_move(self):
        if self.game_over:
            return

        bot = self.white_bot if self.board.turn == chess.WHITE else self.black_bot

        if bot:
            move = bot.choose_move(self.board)
            if move:
                # Check if this is a capture before making the move
                is_capture = self.board.piece_at(move.to_square) is not None
                
                # Make the move
                self.board.push(move)
                self.last_move = move
                
                # Store the engine calculation time
                if hasattr(bot, 'get_last_move_time'):
                    self.last_engine_calc_time = bot.get_last_move_time()
                else:
                    self.last_engine_calc_time = 0

                # Call the appropriate sound callback
                if is_capture and self.on_capture_callback:
                    self.on_capture_callback()
                elif self.on_move_callback:
                    self.on_move_callback()
                
                # Check if the move resulted in check
                if self.board.is_check() and self.on_check_callback:
                    self.on_check_callback()
                
                # Check game ending conditions
                if self.board.is_checkmate() or self.board.is_stalemate() or \
                        self.board.is_insufficient_material() or self.board.is_seventyfive_moves() or \
                        self.board.is_fivefold_repetition():
                    self.game_over = True

    def get_legal_moves(self, square):
        moves = {}
        for move in self.board.legal_moves:
            if move.from_square == square:
                if self.board.piece_at(move.to_square):
                    moves[move.to_square] = "kill"
                else:
                    moves[move.to_square] = "normal"
        return moves

    def is_in_check(self):
        return self.board.is_check()

    def get_king_square(self):
        return self.board.king(self.board.turn)

    def is_human_turn(self):
        if self.mode == GameMode.HUMAN_VS_HUMAN:
            return True
        elif self.mode == GameMode.HUMAN_VS_BOT:
            return self.board.turn == self.human_side
        else:
            return False

    def update(self):
        if self.game_over:
            return

        current_time = time.time()

        if not self.is_human_turn():
            if not self.waiting_for_bot:
                self.waiting_for_bot = True
                self.last_bot_move_time = current_time
            elif current_time - self.last_bot_move_time >= self.bot_move_time:
                self.make_bot_move()
                self.waiting_for_bot = False

    def get_game_state(self):
        if self.game_over:
            if self.board.is_checkmate():
                winner = "Black" if self.board.turn == chess.WHITE else "White"
                return f"Checkmate! {winner} wins!"
            elif self.board.is_stalemate():
                return "Stalemate! Draw!"
            elif self.board.is_insufficient_material():
                return "Insufficient material! Draw!"
            else:
                return "Game over! Draw!"
        elif self.board.is_check():
            return "Check!"
        return ""