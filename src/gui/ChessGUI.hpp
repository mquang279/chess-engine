#ifndef CHESS_GUI_HPP
#define CHESS_GUI_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <iostream>
#include <functional>
#include <vector>

#include "chess.hpp"
#include "engine/ChessEngine.hpp"

// Define player types
enum class PlayerType
{
    HUMAN,
    BOT
};

class ChessGUI
{
public:
    ChessGUI(unsigned int width = 800, unsigned int height = 800);
    ~ChessGUI();

    // Run the GUI loop
    void run();

    // Set the position from a FEN string
    void setPosition(const std::string &fen);

    // Get the current chess board
    chess::Board &getBoard() { return board; }

    // Set player types for white and black
    void setPlayerTypes(PlayerType whitePlayerType, PlayerType blackPlayerType)
    {
        whitePlayer = whitePlayerType;
        blackPlayer = blackPlayerType;
    }

private:
    // Initialize the GUI elements
    void initialize();

    // Load the chess pieces textures
    void loadTextures(const std::string &theme = "tartanian");

    // Draw the board and pieces
    void drawBoard();
    void drawPieces();
    void drawMoveHistory();
    void drawButtons();
    void drawStatusBar();

    // Draw the selection mode screen
    void drawSelectionScreen();

    // Handle mouse events
    void handleMousePressed(sf::Vector2i position);
    void handleMouseReleased(sf::Vector2i position);
    void handleMouseMoved(sf::Vector2i position);
    void handleMouseScrolled(float delta);
    void handleSelectionScreenMousePress(sf::Vector2i position);
    void handleButtonClick(sf::Vector2i position);

    // Add a move to the move history
    void addMoveToHistory(const chess::Move &move);

    // Undo the last move
    void undoLastMove();

    // Reset to selection screen
    void resetToSelectionScreen();

    // Make a computer move using the chess engine
    void makeComputerMove();

    // Check if it's the computer's turn to move
    bool isComputerTurn();

    // Update the status text with current game state
    void updateStatusText();

    // Convert between board coordinates and pixel coordinates
    sf::Vector2f boardToScreen(chess::Square square);
    chess::Square screenToBoard(sf::Vector2f position);

    // SFML elements
    sf::RenderWindow window;
    sf::Font font;

    // Textures and sprites
    sf::Texture pieceTextures[12]; // 6 pieces * 2 colors
    sf::Texture boardTexture;
    sf::Sprite boardSprite;
    sf::Sprite pieceSprites[64]; // One sprite per square

    // Chess logic
    chess::Board board;
    chess::Move currentMove;

    // Chess engine
    ChessEngine engine;

    // GUI state
    bool dragging;
    int draggedPieceIndex;
    sf::Vector2f dragOffset;
    chess::Square fromSquare;

    // Board dimensions
    float squareSize;
    unsigned int boardWidth;
    unsigned int boardHeight;

    // Colors
    sf::Color lightSquareColor;
    sf::Color darkSquareColor;
    sf::Color selectedSquareColor;
    sf::Color legalMoveColor;
    sf::Color lastMoveColor; // Color for highlighting the last move

    // Audio
    sf::SoundBuffer moveSoundBuffer;
    sf::SoundBuffer captureSoundBuffer;
    sf::Sound moveSound;
    sf::Sound captureSound;

    // Button properties
    sf::RectangleShape homeButton;
    sf::RectangleShape backButton;
    sf::Text homeButtonText;
    sf::Text backButtonText;
    sf::Color buttonColor;
    sf::Color buttonHoverColor;
    bool isHomeButtonHovered;
    bool isBackButtonHovered;

    // Status bar
    sf::RectangleShape statusBar;
    sf::Text statusText;

    // Game mode selection
    bool inSelectionMode;
    PlayerType whitePlayer;
    PlayerType blackPlayer;

    // Last move tracking for highlighting
    chess::Square lastMoveFrom;
    chess::Square lastMoveTo;
};

#endif // CHESS_GUI_HPP