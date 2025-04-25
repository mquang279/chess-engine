#include "ChessGUI.hpp"

ChessGUI::ChessGUI(unsigned int width, unsigned int height)
    : window(sf::VideoMode(width, height), "Chess GUI"),
      boardWidth(width),
      boardHeight(height),
      dragging(false),
      draggedPieceIndex(-1),
      fromSquare(chess::Square::NO_SQ),
      whitePlayer(PlayerType::HUMAN),
      blackPlayer(PlayerType::HUMAN),
      inSelectionMode(true),
      lastMoveFrom(chess::Square::NO_SQ),
      lastMoveTo(chess::Square::NO_SQ),
      isHomeButtonHovered(false),
      isBackButtonHovered(false)
{

    // Set board dimensions
    squareSize = std::min(width, height) / 8.0f;

    // Set board colors
    lightSquareColor = sf::Color(240, 217, 181);
    darkSquareColor = sf::Color(181, 136, 99);
    selectedSquareColor = sf::Color(130, 151, 105);
    legalMoveColor = sf::Color(130, 151, 105, 128);
    lastMoveColor = sf::Color(255, 255, 102, 100); // Yellow with some transparency
    buttonColor = sf::Color(130, 151, 105);        // Same as selection color
    buttonHoverColor = sf::Color(100, 120, 80);    // Darker shade for hover effect

    // Initialize the board with the starting position
    board = chess::Board();

    // Initialize GUI components
    initialize();
}

ChessGUI::~ChessGUI()
{
    // Cleanup resources if needed
}

void ChessGUI::initialize()
{
    // Load textures
    loadTextures();

    // Load font
    if (!font.loadFromFile("src/assets/fonts/Roboto-Regular.ttf"))
    {
        std::cerr << "Failed to load font!" << std::endl;
    }

    // Load sounds
    if (!moveSoundBuffer.loadFromFile("src/assets/sounds/Move.ogg"))
    {
        std::cerr << "Failed to load move sound!" << std::endl;
    }

    if (!captureSoundBuffer.loadFromFile("src/assets/sounds/Capture.ogg"))
    {
        std::cerr << "Failed to load capture sound!" << std::endl;
    }

    moveSound.setBuffer(moveSoundBuffer);
    captureSound.setBuffer(captureSoundBuffer);

    // Configure window
    window.setFramerateLimit(60);

    // Define dimensions for UI elements
    float buttonWidth = 120.0f;
    float buttonHeight = 40.0f;
    float buttonMargin = 20.0f;
    float statusBarHeight = 50.0f;

    // Create status bar at the bottom of the screen
    statusBar.setSize(sf::Vector2f(boardWidth, statusBarHeight));
    statusBar.setPosition(0, boardHeight - statusBarHeight);
    statusBar.setFillColor(sf::Color(50, 50, 50)); // Dark gray

    statusText.setFont(font);
    statusText.setCharacterSize(20);
    statusText.setFillColor(sf::Color::White);

    // Update status text with initial game state
    updateStatusText();

    // Home button (left side, above the status bar)
    homeButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    homeButton.setPosition(buttonMargin, boardHeight - statusBarHeight - buttonHeight - buttonMargin);
    homeButton.setFillColor(buttonColor);

    homeButtonText.setFont(font);
    homeButtonText.setString("Home");
    homeButtonText.setCharacterSize(18);
    homeButtonText.setFillColor(sf::Color::White);

    sf::FloatRect homeTextBounds = homeButtonText.getLocalBounds();
    homeButtonText.setPosition(
        homeButton.getPosition().x + (buttonWidth - homeTextBounds.width) / 2,
        homeButton.getPosition().y + (buttonHeight - homeTextBounds.height) / 2 - homeTextBounds.top);

    // Back button (right side, above the status bar)
    backButton.setSize(sf::Vector2f(buttonWidth, buttonHeight));
    backButton.setPosition(boardWidth - buttonWidth - buttonMargin, boardHeight - statusBarHeight - buttonHeight - buttonMargin);
    backButton.setFillColor(buttonColor);

    backButtonText.setFont(font);
    backButtonText.setString("Back");
    backButtonText.setCharacterSize(18);
    backButtonText.setFillColor(sf::Color::White);

    sf::FloatRect backTextBounds = backButtonText.getLocalBounds();
    backButtonText.setPosition(
        backButton.getPosition().x + (buttonWidth - backTextBounds.width) / 2,
        backButton.getPosition().y + (buttonHeight - backTextBounds.height) / 2 - backTextBounds.top);
}

void ChessGUI::loadTextures(const std::string &theme)
{
    // Load each piece texture
    const std::string pieces[] = {"P", "N", "B", "R", "Q", "K"};
    const std::string colors[] = {"w", "b"};

    for (int c = 0; c < 2; c++)
    {
        for (int p = 0; p < 6; p++)
        {
            int index = c * 6 + p;
            std::string filename = "src/assets/themes/" + theme + "/" + colors[c] + pieces[p] + ".png";
            if (!pieceTextures[index].loadFromFile(filename))
            {
                std::cerr << "Failed to load texture: " << filename << std::endl;
            }
            pieceTextures[index].setSmooth(true);
        }
    }
}

void ChessGUI::run()
{
    // Create a clock to control the delay between AI moves
    sf::Clock aiDelayClock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    if (inSelectionMode)
                    {
                        handleSelectionScreenMousePress(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    }
                    else
                    {
                        handleMousePressed(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    }
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left && !inSelectionMode)
                {
                    handleMouseReleased(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                }
            }
            else if (event.type == sf::Event::MouseMoved && !inSelectionMode)
            {
                handleMouseMoved(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            }
        }

        // Check if it's the computer's turn and make a move after a small delay
        // Only do this if we're not in selection mode and we're not currently dragging a piece
        if (!inSelectionMode && !dragging && isComputerTurn() && aiDelayClock.getElapsedTime().asMilliseconds() > 500)
        {
            makeComputerMove();
            aiDelayClock.restart(); // Reset the clock after making a move
        }

        // Clear the window
        window.clear(sf::Color(40, 40, 40));

        // Draw selection screen or chess board based on the mode
        if (inSelectionMode)
        {
            drawSelectionScreen();
        }
        else
        {
            // Draw chess board and pieces
            drawBoard();
            drawPieces();
            drawButtons();
        }

        // Display the window
        window.display();
    }
}

void ChessGUI::drawBoard()
{
    // Draw the chess board
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
            square.setPosition(file * squareSize, (7 - rank) * squareSize);

            // Get the square index
            int squareIndex = rank * 8 + file;
            chess::Square currentSquare = static_cast<chess::Square>(squareIndex);

            // Set the color of the square
            if ((rank + file) % 2 == 0)
            {
                square.setFillColor(lightSquareColor);
            }
            else
            {
                square.setFillColor(darkSquareColor);
            }

            // Draw the square
            window.draw(square);

            // Highlight last move (from square and to square)
            if (lastMoveFrom != chess::Square::NO_SQ && lastMoveTo != chess::Square::NO_SQ)
            {
                if (currentSquare == lastMoveFrom || currentSquare == lastMoveTo)
                {
                    sf::RectangleShape highlightSquare(sf::Vector2f(squareSize, squareSize));
                    highlightSquare.setPosition(file * squareSize, (7 - rank) * squareSize);
                    highlightSquare.setFillColor(lastMoveColor);
                    window.draw(highlightSquare);
                }
            }

            // Draw coordinates on the board
            if (rank == 0 || file == 0)
            {
                sf::Text text;
                text.setFont(font);
                text.setCharacterSize(12);
                text.setFillColor(sf::Color(255, 255, 255, 180));

                if (file == 0)
                {
                    text.setString(std::to_string(rank + 1));
                    text.setPosition(5, (7 - rank) * squareSize + 5);
                }

                if (rank == 0)
                {
                    text.setString(std::string(1, 'a' + file));
                    text.setPosition(file * squareSize + squareSize - 15, 7 * squareSize + squareSize - 20);
                }

                window.draw(text);
            }
        }
    }
}

void ChessGUI::drawPieces()
{
    // Draw all chess pieces based on the board state
    for (int sq = 0; sq < 64; sq++)
    {
        chess::Square square = static_cast<chess::Square>(sq);
        chess::Piece piece = board.at(square);

        if (piece != chess::Piece::NONE && sq != draggedPieceIndex)
        {
            sf::Sprite sprite;
            int textureIndex;

            // Get the appropriate texture index
            if (piece <= chess::Piece::WHITEKING)
            {
                // White pieces
                textureIndex = static_cast<int>(piece);
            }
            else
            {
                // Black pieces (offset by 6)
                textureIndex = static_cast<int>(piece) - 6;
                textureIndex += 6; // Black texture offset
            }

            sprite.setTexture(pieceTextures[textureIndex]);
            sf::Vector2f pos = boardToScreen(square);
            sprite.setPosition(pos);

            // Scale the sprite to fit the square
            float scale = squareSize / sprite.getTexture()->getSize().x;
            sprite.setScale(scale, scale);

            window.draw(sprite);
        }
    }

    // Draw the dragged piece (if any) on top
    if (dragging && draggedPieceIndex >= 0)
    {
        chess::Piece piece = board.at(static_cast<chess::Square>(draggedPieceIndex));
        if (piece != chess::Piece::NONE)
        {
            sf::Sprite sprite;
            int textureIndex;

            // Get the appropriate texture index
            if (piece <= chess::Piece::WHITEKING)
            {
                // White pieces
                textureIndex = static_cast<int>(piece);
            }
            else
            {
                // Black pieces (offset by 6)
                textureIndex = static_cast<int>(piece) - 6;
                textureIndex += 6; // Black texture offset
            }

            sprite.setTexture(pieceTextures[textureIndex]);
            sprite.setPosition(dragOffset);

            // Scale the sprite to fit the square
            float scale = squareSize / sprite.getTexture()->getSize().x;
            sprite.setScale(scale, scale);

            window.draw(sprite);
        }
    }
}

void ChessGUI::handleMousePressed(sf::Vector2i position)
{
    sf::Vector2f boardPos(position.x, position.y);
    chess::Square square = screenToBoard(boardPos);

    if (square != chess::Square::NO_SQ)
    {
        chess::Piece piece = board.at(square);

        // Only allow dragging pieces of the current player's color
        if (piece != chess::Piece::NONE &&
            ((board.sideToMove() == chess::Color::WHITE && piece <= chess::Piece::WHITEKING) ||
             (board.sideToMove() == chess::Color::BLACK && piece >= chess::Piece::BLACKPAWN)))
        {

            dragging = true;
            draggedPieceIndex = static_cast<int>(square);
            fromSquare = square;
            dragOffset = sf::Vector2f(position.x - (boardPos.x / squareSize) * squareSize,
                                      position.y - (boardPos.y / squareSize) * squareSize);
        }
    }
}

void ChessGUI::handleMouseReleased(sf::Vector2i position)
{
    if (dragging)
    {
        sf::Vector2f boardPos(position.x, position.y);
        chess::Square toSquare = screenToBoard(boardPos);

        if (fromSquare != chess::Square::NO_SQ && toSquare != chess::Square::NO_SQ && fromSquare != toSquare)
        {
            // Generate legal moves for the current position
            chess::Movelist moves;
            chess::movegen::legalmoves(moves, board);

            // Find if the move is legal
            for (int i = 0; i < moves.size(); i++)
            {
                chess::Move move = moves[i];
                if (move.from() == fromSquare && move.to() == toSquare)
                {
                    // Handle promotion
                    if (move.typeOf() == chess::Move::PROMOTION)
                    {
                        // Default to queen for now (could add UI for selection later)
                        move = chess::Move::make<chess::Move::PROMOTION>(move.from(), move.to(), chess::PieceType::QUEEN);
                    }

                    // Check if it's a capture
                    bool isCapture = board.isCapture(move);

                    // Store the last move squares for highlighting
                    lastMoveFrom = fromSquare;
                    lastMoveTo = toSquare;

                    // Make the move
                    board.makeMove(move);

                    // Update the status text after the move
                    updateStatusText();

                    // Play appropriate sound
                    if (isCapture)
                    {
                        captureSound.play();
                    }
                    else
                    {
                        moveSound.play();
                    }

                    break;
                }
            }
        }

        // Reset dragging state
        dragging = false;
        draggedPieceIndex = -1;
        fromSquare = chess::Square::NO_SQ;
    }
}

void ChessGUI::handleMouseMoved(sf::Vector2i position)
{
    if (dragging)
    {
        dragOffset = sf::Vector2f(position.x - (squareSize / 2), position.y - (squareSize / 2));
    }
}

sf::Vector2f ChessGUI::boardToScreen(chess::Square square)
{
    int sq = static_cast<int>(square);
    int file = sq % 8;
    int rank = sq / 8;
    return sf::Vector2f(file * squareSize, (7 - rank) * squareSize);
}

chess::Square ChessGUI::screenToBoard(sf::Vector2f position)
{
    int file = static_cast<int>(position.x / squareSize);
    int rank = 7 - static_cast<int>(position.y / squareSize);

    if (file >= 0 && file < 8 && rank >= 0 && rank < 8)
    {
        return static_cast<chess::Square>(rank * 8 + file);
    }

    return chess::Square::NO_SQ;
}

void ChessGUI::setPosition(const std::string &fen)
{
    board.setFen(fen);
}

void ChessGUI::drawSelectionScreen()
{
    // Draw background
    sf::RectangleShape background(sf::Vector2f(boardWidth, boardHeight));
    background.setFillColor(sf::Color(40, 40, 40));
    window.draw(background);

    // Calculate positions for title and buttons
    float centerX = boardWidth / 2;
    float titleY = boardHeight * 0.2;
    float firstButtonY = boardHeight * 0.4;
    float buttonSpacing = boardHeight * 0.15;

    // Title text
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("Select Game Mode");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color(255, 255, 255));

    // Center the title text
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(centerX - titleBounds.width / 2, titleY - titleBounds.height / 2);
    window.draw(titleText);

    // Create buttons for game modes
    sf::RectangleShape button1(sf::Vector2f(boardWidth * 0.6f, boardHeight * 0.1f));
    sf::RectangleShape button2(sf::Vector2f(boardWidth * 0.6f, boardHeight * 0.1f));
    sf::RectangleShape button3(sf::Vector2f(boardWidth * 0.6f, boardHeight * 0.1f));

    button1.setPosition(centerX - button1.getSize().x / 2, firstButtonY);
    button2.setPosition(centerX - button2.getSize().x / 2, firstButtonY + buttonSpacing);
    button3.setPosition(centerX - button3.getSize().x / 2, firstButtonY + buttonSpacing * 2);

    button1.setFillColor(sf::Color(130, 151, 105));
    button2.setFillColor(sf::Color(130, 151, 105));
    button3.setFillColor(sf::Color(130, 151, 105));

    window.draw(button1);
    window.draw(button2);
    window.draw(button3);

    // Button texts
    sf::Text button1Text, button2Text, button3Text;
    button1Text.setFont(font);
    button2Text.setFont(font);
    button3Text.setFont(font);

    button1Text.setString("Human vs Bot");
    button2Text.setString("Human vs Human");
    button3Text.setString("Bot vs Bot");

    button1Text.setCharacterSize(24);
    button2Text.setCharacterSize(24);
    button3Text.setCharacterSize(24);

    button1Text.setFillColor(sf::Color::White);
    button2Text.setFillColor(sf::Color::White);
    button3Text.setFillColor(sf::Color::White);

    // Center texts in buttons
    sf::FloatRect button1TextBounds = button1Text.getLocalBounds();
    sf::FloatRect button2TextBounds = button2Text.getLocalBounds();
    sf::FloatRect button3TextBounds = button3Text.getLocalBounds();

    button1Text.setPosition(
        button1.getPosition().x + (button1.getSize().x - button1TextBounds.width) / 2,
        button1.getPosition().y + (button1.getSize().y - button1TextBounds.height) / 2 - button1TextBounds.top);

    button2Text.setPosition(
        button2.getPosition().x + (button2.getSize().x - button2TextBounds.width) / 2,
        button2.getPosition().y + (button2.getSize().y - button2TextBounds.height) / 2 - button2TextBounds.top);

    button3Text.setPosition(
        button3.getPosition().x + (button3.getSize().x - button3TextBounds.width) / 2,
        button3.getPosition().y + (button3.getSize().y - button3TextBounds.height) / 2 - button3TextBounds.top);

    window.draw(button1Text);
    window.draw(button2Text);
    window.draw(button3Text);
}

void ChessGUI::handleSelectionScreenMousePress(sf::Vector2i position)
{
    // Calculate button positions (matching the ones in drawSelectionScreen)
    float centerX = boardWidth / 2;
    float firstButtonY = boardHeight * 0.4;
    float buttonSpacing = boardHeight * 0.15;

    float buttonWidth = boardWidth * 0.6f;
    float buttonHeight = boardHeight * 0.1f;

    // Check if the click is within any of the buttons
    // Human vs Bot
    if (position.x >= centerX - buttonWidth / 2 &&
        position.x <= centerX + buttonWidth / 2 &&
        position.y >= firstButtonY &&
        position.y <= firstButtonY + buttonHeight)
    {
        whitePlayer = PlayerType::HUMAN;
        blackPlayer = PlayerType::BOT;
        inSelectionMode = false;
    }
    // Human vs Human
    else if (position.x >= centerX - buttonWidth / 2 &&
             position.x <= centerX + buttonWidth / 2 &&
             position.y >= firstButtonY + buttonSpacing &&
             position.y <= firstButtonY + buttonSpacing + buttonHeight)
    {
        whitePlayer = PlayerType::HUMAN;
        blackPlayer = PlayerType::HUMAN;
        inSelectionMode = false;
    }
    // Bot vs Bot
    else if (position.x >= centerX - buttonWidth / 2 &&
             position.x <= centerX + buttonWidth / 2 &&
             position.y >= firstButtonY + buttonSpacing * 2 &&
             position.y <= firstButtonY + buttonSpacing * 2 + buttonHeight)
    {
        whitePlayer = PlayerType::BOT;
        blackPlayer = PlayerType::BOT;
        inSelectionMode = false;
    }
}

bool ChessGUI::isComputerTurn()
{
    if (inSelectionMode)
        return false;

    if (board.sideToMove() == chess::Color::WHITE)
    {
        return (whitePlayer == PlayerType::BOT);
    }
    else
    {
        return (blackPlayer == PlayerType::BOT);
    }
}

void ChessGUI::makeComputerMove()
{
    // Use the chess engine to get the best move
    chess::Move bestMove = engine.getBestMove(board);

    if (bestMove != chess::Move::NULL_MOVE)
    {
        // Store the last move squares for highlighting
        lastMoveFrom = bestMove.from();
        lastMoveTo = bestMove.to();

        // Check if it's a capture
        bool isCapture = board.isCapture(bestMove);

        // Make the move
        board.makeMove(bestMove);

        // Update status text after move
        updateStatusText();

        // Play appropriate sound
        if (isCapture)
        {
            captureSound.play();
        }
        else
        {
            moveSound.play();
        }
    }
}

void ChessGUI::updateStatusText()
{
    std::string turnText = (board.sideToMove() == chess::Color::WHITE) ? "White" : "Black";
    std::string checkText = board.inCheck() ? " - CHECK!" : "";

    statusText.setString(turnText + "'s turn" + checkText);

    // Center the text in the status bar
    sf::FloatRect textBounds = statusText.getLocalBounds();
    statusText.setPosition(
        statusBar.getPosition().x + (statusBar.getSize().x - textBounds.width) / 2,
        statusBar.getPosition().y + (statusBar.getSize().y - textBounds.height) / 2 - textBounds.top);

    // Make text red if in check
    if (board.inCheck())
    {
        statusText.setFillColor(sf::Color::Red);
    }
    else
    {
        statusText.setFillColor(sf::Color::White);
    }
}

void ChessGUI::drawStatusBar()
{
    // Draw the status bar and its text
    window.draw(statusBar);
    window.draw(statusText);
}

void ChessGUI::drawButtons()
{
    // Update button colors based on hover state
    homeButton.setFillColor(isHomeButtonHovered ? buttonHoverColor : buttonColor);
    backButton.setFillColor(isBackButtonHovered ? buttonHoverColor : buttonColor);

    // Draw buttons and their text
    window.draw(homeButton);
    window.draw(homeButtonText);
    window.draw(backButton);
    window.draw(backButtonText);

    // Draw status bar
    drawStatusBar();
}