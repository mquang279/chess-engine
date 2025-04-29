from enum import Enum

import chess
import pygame
import os
import sys

# Add project root to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Direct imports from the ui directory
from ui.ChessGame import ChessGame
from ui.GameMode import GameMode

class UI:
    def __init__(self, width=600, height=700):
        pygame.init()
        # Initialize pygame mixer for sound effects
        pygame.mixer.init()
        
        self.WIDTH = width
        self.HEIGHT = height
        self.screen = pygame.display.set_mode((self.WIDTH, self.HEIGHT))
        pygame.display.set_caption("Chess 36")
        self.game = None
        self.square_size = self.WIDTH // 8
        self.highlighted_squares = {}
        self.piece_images = {}
        
        # Load sound effects
        self.move_sound = pygame.mixer.Sound("assets/sounds/Move.ogg")
        self.capture_sound = pygame.mixer.Sound("assets/sounds/Capture.ogg")
        self.notify_sound = pygame.mixer.Sound("assets/sounds/GenericNotify.ogg")
        
        # Use the tartanian theme for chess pieces
        theme_path = "assets/themes/tartanian"
        # Map python-chess piece symbols to file names
        symbol_to_file = {
            'p': 'bP', 'n': 'bN', 'b': 'bB', 'r': 'bR', 'q': 'bQ', 'k': 'bK',
            'P': 'wP', 'N': 'wN', 'B': 'wB', 'R': 'wR', 'Q': 'wQ', 'K': 'wK'
        }
        
        for piece in chess.PIECE_SYMBOLS[1:]:
            self.piece_images[piece] = pygame.image.load(os.path.join(theme_path, f"{symbol_to_file[piece]}.png"))
            self.piece_images[piece.upper()] = pygame.image.load(os.path.join(theme_path, f"{symbol_to_file[piece.upper()]}.png"))

        # Create simple highlight images if not present
        self.highlight_image = pygame.Surface((self.square_size, self.square_size), pygame.SRCALPHA)
        pygame.draw.rect(self.highlight_image, (0, 255, 0, 255), self.highlight_image.get_rect(), 2)
        
        self.kill_highlight_image = pygame.Surface((self.square_size, self.square_size), pygame.SRCALPHA)
        pygame.draw.rect(self.kill_highlight_image, (255, 0, 0, 255), self.kill_highlight_image.get_rect(), 2)
        
        self.check_highlight_image = pygame.Surface((self.square_size, self.square_size), pygame.SRCALPHA)
        pygame.draw.rect(self.check_highlight_image, (255, 255, 0, 255), self.check_highlight_image.get_rect(), 2)

        self.colors = [pygame.Color(77,83,88), pygame.Color(106,114,120)]
        self.button_color = pygame.Color("lightblue")
        self.hover_color = pygame.Color("skyblue")
        self.text_color = pygame.Color("black")
        
        # Added flag for side selection screen and player color preference
        self.show_side_selection = False
        self.player_color = chess.WHITE  # Default to white
        self.selected_mode = None  # Store the selected game mode

        self.font = pygame.font.SysFont('Arial', 24)
        self.small_font = pygame.font.SysFont('Arial', 18)

        button_height = 50
        button_margin = 10

        self.mode_buttons = [
        {
            'rect': pygame.Rect(width//2 - 150, height//2 - 100,
                               300, button_height),
            'text': "Human vs Human",
            'mode': GameMode.HUMAN_VS_HUMAN
        },
        {
            'rect': pygame.Rect(width//2 - 150, height//2,
                               300, button_height),
            'text': "Human vs Bot",
            'mode': GameMode.HUMAN_VS_BOT
        },
        {
            'rect': pygame.Rect(width//2 - 150, height//2 + 100,
                               300, button_height),
            'text': "Bot vs Bot",
            'mode': GameMode.BOT_VS_BOT
        }
    ]

        self.side_selection_buttons = [
            {
                'rect': pygame.Rect(width//2 - 150, height//2 - 50,
                                   300, button_height),
                'text': "Play as White",
                'color': chess.WHITE
            },
            {
                'rect': pygame.Rect(width//2 - 150, height//2 + 50,
                                   300, button_height),
                'text': "Play as Black",
                'color': chess.BLACK
            }
        ]

        self.show_mode_selection = True
        self.restart_button = pygame.Rect(width // 2 - 75, self.square_size * 8 + 25, 150, 40)


    def render_mode_selection(self):
        self.screen.fill(pygame.Color("darkgray"))

        title = self.font.render("Select Game Mode", True, self.text_color)
        title_rect = title.get_rect(center=(self.WIDTH // 2, 100))
        self.screen.blit(title, title_rect)

        mouse_pos = pygame.mouse.get_pos()
        for button in self.mode_buttons:
            color = self.hover_color if button['rect'].collidepoint(mouse_pos) else self.button_color

            pygame.draw.rect(self.screen, color, button['rect'], border_radius=5)

            text = self.font.render(button['text'], True, self.text_color)
            text_rect = text.get_rect(center=button['rect'].center)
            self.screen.blit(text, text_rect)

        pygame.display.flip()

    def render_side_selection(self):
        self.screen.fill(pygame.Color("darkgray"))

        title = self.font.render("Choose Your Side", True, self.text_color)
        title_rect = title.get_rect(center=(self.WIDTH // 2, 100))
        self.screen.blit(title, title_rect)

        mouse_pos = pygame.mouse.get_pos()
        for button in self.side_selection_buttons:
            color = self.hover_color if button['rect'].collidepoint(mouse_pos) else self.button_color

            pygame.draw.rect(self.screen, color, button['rect'], border_radius=5)

            text = self.font.render(button['text'], True, self.text_color)
            text_rect = text.get_rect(center=button['rect'].center)
            self.screen.blit(text, text_rect)

        pygame.display.flip()

    def render_game(self):
        self.screen.fill(pygame.Color("darkgray"))

        for row in range(8):
            for col in range(8):
                color = self.colors[(row + col) % 2]
                pygame.draw.rect(self.screen, color, pygame.Rect(
                    col * self.square_size,
                    row * self.square_size,
                    self.square_size,
                    self.square_size
                ))

                square = chess.square(col, 7 - row)

                if self.game.last_move and (
                        square == self.game.last_move.from_square or square == self.game.last_move.to_square):
                    pygame.draw.rect(self.screen, pygame.Color(56,62,86), pygame.Rect(
                        col * self.square_size,
                        row * self.square_size,
                        self.square_size,
                        self.square_size
                    ))

                if square in self.highlighted_squares:
                    highlight_type = self.highlighted_squares[square]
                    img = self.highlight_image
                    if highlight_type == "kill":
                        img = self.kill_highlight_image
                    elif highlight_type == "check":
                        img = self.check_highlight_image

                    self.screen.blit(pygame.transform.scale(img, (self.square_size, self.square_size)),
                                     (col * self.square_size, row * self.square_size))

                piece = self.game.board.piece_at(square)
                if piece:
                    img = self.piece_images[piece.symbol()]
                    self.screen.blit(pygame.transform.scale(img, (self.square_size, self.square_size)),
                                     (col * self.square_size, row * self.square_size))



        state_text = self.game.get_game_state()
        if state_text:
            text_surface = self.font.render(state_text, True, pygame.Color("black"))
            self.screen.blit(text_surface, (10, 8 * self.square_size + 10))

        turn_text = "White to move" if self.game.board.turn == chess.WHITE else "Black to move"
        turn_surface = self.small_font.render(turn_text, True, pygame.Color("black"))
        self.screen.blit(turn_surface, (self.WIDTH - turn_surface.get_width() - 10, 8 * self.square_size + 10))

        # Display engine calculation time if this is a game with a bot
        if self.game.mode != GameMode.HUMAN_VS_HUMAN and hasattr(self.game, 'last_engine_calc_time') and self.game.last_engine_calc_time > 0:
            time_text = f"Engine time: {self.game.last_engine_calc_time:.4f} sec"
            time_surface = self.small_font.render(time_text, True, pygame.Color("black"))
            self.screen.blit(time_surface, (10, 8 * self.square_size + 35))

        mouse_pos = pygame.mouse.get_pos()
        button_color = self.hover_color if self.restart_button.collidepoint(mouse_pos) else self.button_color

        pygame.draw.rect(self.screen, button_color, self.restart_button, border_radius=5)
        restart_text = self.font.render("Restart", True, self.text_color)
        restart_text_rect = restart_text.get_rect(center=self.restart_button.center)
        self.screen.blit(restart_text, restart_text_rect)

        pygame.display.flip()


    def get_square_from_pos(self, pos):
        col = pos[0] // self.square_size
        row = pos[1] // self.square_size


        if 0 <= col < 8 and 0 <= row < 8:
            return chess.square(col, 7 - row)
        return None

    def highlight_moves(self, square):
        self.highlighted_squares = self.game.get_legal_moves(square)

        if self.game.is_in_check():
            king_square = self.game.get_king_square()
            self.highlighted_squares[king_square] = "check"

    def handle_mode_selection(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            pos = pygame.mouse.get_pos()
            for button in self.mode_buttons:
                if button['rect'].collidepoint(pos):
                    if button['mode'] == GameMode.HUMAN_VS_BOT:
                        self.selected_mode = button['mode']
                        self.show_side_selection = True
                        self.show_mode_selection = False
                    else:
                        self.game = ChessGame(mode=button['mode'])
                        self.show_mode_selection = False
                    break

    def handle_side_selection(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            pos = pygame.mouse.get_pos()
            for button in self.side_selection_buttons:
                if button['rect'].collidepoint(pos):
                    self.player_color = button['color']
                    self.game = ChessGame(mode=self.selected_mode, human_side=self.player_color)
                    self.show_side_selection = False
                    break

    def play_move_sound(self, is_capture):
        """Play sound effect based on move type"""
        if is_capture:
            self.capture_sound.play()
        else:
            self.move_sound.play()
    
    def play_notify_sound(self):
        """Play notification sound for events like check"""
        self.notify_sound.play()

    def handle_game_input(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            pos = pygame.mouse.get_pos()

            if self.restart_button.collidepoint(pos):
                self.show_mode_selection = True
                return

            if not self.game.is_human_turn():
                return

            square = self.get_square_from_pos(pos)
            if square is not None:
                if self.game.selected_square is None:
                    piece = self.game.board.piece_at(square)
                    if piece and piece.color == self.game.board.turn:
                        self.game.selected_square = square
                        self.highlight_moves(square)
                else:
                    # Check if the move is a capture
                    is_capture = self.game.board.piece_at(square) is not None
                    
                    if self.game.make_move(self.game.selected_square, square):
                        # Play appropriate sound for the move
                        self.play_move_sound(is_capture)
                        
                        # Play check notification if applicable
                        if self.game.is_in_check():
                            self.play_notify_sound()
                        
                        self.game.selected_square = None
                        self.highlighted_squares = {}
                    else:
                        piece = self.game.board.piece_at(square)
                        if piece and piece.color == self.game.board.turn:
                            self.game.selected_square = square
                            self.highlight_moves(square)
                        else:
                            self.game.selected_square = None
                            self.highlighted_squares = {}

    def run(self):
        running = True
        clock = pygame.time.Clock()

        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                if self.show_mode_selection:
                    self.handle_mode_selection(event)
                elif self.show_side_selection:
                    self.handle_side_selection(event)
                else:
                    self.handle_game_input(event)

            if self.show_mode_selection:
                self.render_mode_selection()
            elif self.show_side_selection:
                self.render_side_selection()
            else:
                self.game.update()
                self.render_game()

            clock.tick(60)

        pygame.quit()

