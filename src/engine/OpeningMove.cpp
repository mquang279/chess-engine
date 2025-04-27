#include "OpeningMove.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <random>
#include <algorithm>

OpeningMove::OpeningMove()
    : openingBook(std::make_unique<OpeningNode>())
{
}

bool OpeningMove::initialize(const std::string &openingDirPath)
{
    bool foundAnyFiles = false;

    try
    {
        // Check if directory exists
        if (!std::filesystem::exists(openingDirPath) || !std::filesystem::is_directory(openingDirPath))
        {
            std::cerr << "Opening book directory not found: " << openingDirPath << std::endl;
            return false;
        }

        // Iterate through all .pgn files in the directory
        for (const auto &entry : std::filesystem::directory_iterator(openingDirPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".pgn")
            {
                std::cout << "Loading opening book from " << entry.path().string() << std::endl;
                if (parsePgnFile(entry.path().string()))
                {
                    foundAnyFiles = true;
                }
            }
        }

        if (!foundAnyFiles)
        {
            std::cerr << "No PGN files found in " << openingDirPath << std::endl;
            return false;
        }

        std::cout << "Opening book initialized with positions: "
                  << openingBook->positions.size() << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error initializing opening book: " << e.what() << std::endl;
        return false;
    }
}

bool OpeningMove::initializeFromFile(const std::string &pgnFilePath)
{
    // Clear any existing opening book data
    openingBook->positions.clear();

    // Check if file exists
    std::ifstream fileCheck(pgnFilePath);
    if (!fileCheck.good())
    {
        std::cerr << "Opening book file not found: " << pgnFilePath << std::endl;
        return false;
    }
    fileCheck.close();

    std::cout << "Loading opening book from specific file: " << pgnFilePath << std::endl;
    bool success = parsePgnFile(pgnFilePath);

    if (success)
    {
        std::cout << "Opening book initialized with positions: "
                  << openingBook->positions.size() << std::endl;
    }

    return success;
}

bool OpeningMove::parsePgnFile(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open PGN file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    std::string moveText;
    bool inMoveText = false;

    while (std::getline(file, line))
    {
        // Skip header lines or empty lines
        if (line.empty() || line[0] == '[')
        {
            if (inMoveText && !moveText.empty())
            {
                // Process the previous game
                addGameToBook(moveText);
                moveText.clear();
                inMoveText = false;
            }
            continue;
        }

        // We're in the move text section
        inMoveText = true;
        moveText += line + " ";

        // If line ends with game result, we've reached the end of a game
        if (line.find("1-0") != std::string::npos ||
            line.find("0-1") != std::string::npos ||
            line.find("1/2-1/2") != std::string::npos ||
            line.find("*") != std::string::npos)
        {

            addGameToBook(moveText);
            moveText.clear();
            inMoveText = false;
        }
    }

    // Process the last game if it wasn't processed
    if (inMoveText && !moveText.empty())
    {
        addGameToBook(moveText);
    }

    return true;
}

void OpeningMove::addGameToBook(const std::string &moveText)
{
    // Regular expression to extract moves from PGN format
    std::regex movePattern(R"(([a-zA-Z0-9][a-zA-Z0-9\+\#\=\-]+)[\s\.\[\]\(\)\{\}]*)");

    // Setup initial board
    chess::Board board;

    // Extract moves
    std::sregex_iterator iter(moveText.begin(), moveText.end(), movePattern);
    std::sregex_iterator end;

    int moveCount = 0;

    while (iter != end && moveCount < maxBookMoves)
    {
        std::string moveStr = (*iter)[1];

        // Skip move numbers, annotations and game results
        if (std::isdigit(moveStr[0]) || moveStr == "1-0" || moveStr == "0-1" || moveStr == "1/2-1/2" || moveStr == "*")
        {
            ++iter;
            continue;
        }

        // Convert algebraic notation to Move
        chess::Move move = algebraicToMove(board, moveStr);
        if (move == chess::Move::NULL_MOVE)
        {
            // Failed to parse move, skip to next game
            std::cerr << "Failed to parse move: " << moveStr << std::endl;
            break;
        }

        // Store the move in the opening book
        uint64_t hash = board.hash();
        auto &positionMoves = openingBook->positions[hash];

        // Check if move already exists
        auto it = std::find_if(positionMoves.begin(), positionMoves.end(),
                               [&move](const auto &pair)
                               { return pair.first == move; });

        if (it != positionMoves.end())
        {
            // Increment the weight
            it->second++;
        }
        else
        {
            // Add new move with weight 1
            positionMoves.emplace_back(move, 1);
        }

        // Make the move and continue
        board.makeMove(move);
        moveCount++;
        ++iter;
    }
}

chess::Move OpeningMove::getBookMove(const chess::Board &board)
{
    // Get position hash
    uint64_t hash = board.hash();

    // Check if position is in opening book
    auto it = openingBook->positions.find(hash);
    if (it == openingBook->positions.end())
    {
        return chess::Move::NULL_MOVE;
    }

    const auto &moves = it->second;

    // Find the move with the highest weight (most frequent appearance)
    chess::Move bestMove = chess::Move::NULL_MOVE;
    int highestWeight = 0;

    for (const auto &[move, weight] : moves)
    {
        if (weight > highestWeight)
        {
            highestWeight = weight;
            bestMove = move;
        }
    }

    if (bestMove != chess::Move::NULL_MOVE)
    {
        std::cout << "Using most frequent opening move (weight: " << highestWeight << ")" << std::endl;
    }

    return bestMove;
}

chess::Move OpeningMove::algebraicToMove(const chess::Board &board, const std::string &moveStr)
{
    // Generate all legal moves
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // Check if the move is castling
    if (moveStr == "O-O" || moveStr == "0-0")
    {
        // Kingside castling
        for (const auto &move : moves)
        {
            if (move.typeOf() == chess::Move::CASTLING &&
                chess::utils::squareFile(move.to()) > chess::utils::squareFile(move.from()))
            {
                return move;
            }
        }
    }
    else if (moveStr == "O-O-O" || moveStr == "0-0-0")
    {
        // Queenside castling
        for (const auto &move : moves)
        {
            if (move.typeOf() == chess::Move::CASTLING &&
                chess::utils::squareFile(move.to()) < chess::utils::squareFile(move.from()))
            {
                return move;
            }
        }
    }
    else
    {
        // Handle regular moves
        std::string sanitized = moveStr;
        // Remove check/mate symbols and captures
        sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
                                       [](char c)
                                       { return c == '+' || c == '#' || c == 'x'; }),
                        sanitized.end());

        char targetPiece = 'P'; // Default to pawn
        if (std::isupper(sanitized[0]))
        {
            targetPiece = sanitized[0];
            sanitized = sanitized.substr(1);
        }

        // Handle pawn promotion
        chess::PieceType promotionType = chess::PieceType::NONE;
        size_t equalPos = sanitized.find('=');
        if (equalPos != std::string::npos)
        {
            char promotionChar = sanitized[equalPos + 1];
            switch (promotionChar)
            {
            case 'Q':
                promotionType = chess::PieceType::QUEEN;
                break;
            case 'R':
                promotionType = chess::PieceType::ROOK;
                break;
            case 'B':
                promotionType = chess::PieceType::BISHOP;
                break;
            case 'N':
                promotionType = chess::PieceType::KNIGHT;
                break;
            default:
                break;
            }
            sanitized = sanitized.substr(0, equalPos);
        }

        // Extract destination square
        std::string destSquare;
        if (sanitized.size() >= 2)
        {
            destSquare = sanitized.substr(sanitized.size() - 2);
            sanitized = sanitized.substr(0, sanitized.size() - 2);
        }

        // Convert destination to square index
        int destFile = destSquare[0] - 'a';
        int destRank = destSquare[1] - '1';

        if (destFile < 0 || destFile > 7 || destRank < 0 || destRank > 7)
        {
            return chess::Move::NULL_MOVE;
        }

        chess::Square destSq = chess::Square(destRank * 8 + destFile);

        // Extract disambiguation (if any)
        char disambFile = -1;
        int disambRank = -1;

        if (!sanitized.empty())
        {
            for (char c : sanitized)
            {
                if (c >= 'a' && c <= 'h')
                {
                    disambFile = c - 'a';
                }
                else if (c >= '1' && c <= '8')
                {
                    disambRank = c - '1';
                }
            }
        }

        // Find the matching move
        for (const auto &move : moves)
        {
            chess::Square fromSq = move.from();
            chess::Square toSq = move.to();
            chess::PieceType pieceType = board.at<chess::PieceType>(fromSq);

            // Check if the piece type matches
            bool pieceMatches = false;
            switch (targetPiece)
            {
            case 'P':
                pieceMatches = (pieceType == chess::PieceType::PAWN);
                break;
            case 'N':
                pieceMatches = (pieceType == chess::PieceType::KNIGHT);
                break;
            case 'B':
                pieceMatches = (pieceType == chess::PieceType::BISHOP);
                break;
            case 'R':
                pieceMatches = (pieceType == chess::PieceType::ROOK);
                break;
            case 'Q':
                pieceMatches = (pieceType == chess::PieceType::QUEEN);
                break;
            case 'K':
                pieceMatches = (pieceType == chess::PieceType::KING);
                break;
            }

            if (!pieceMatches)
            {
                continue;
            }

            // Check destination
            if (toSq != destSq)
            {
                continue;
            }

            // Check disambiguation
            if (disambFile != -1 && static_cast<int>(chess::utils::squareFile(fromSq)) != disambFile)
            {
                continue;
            }

            if (disambRank != -1 && static_cast<int>(chess::utils::squareRank(fromSq)) != disambRank)
            {
                continue;
            }

            // Check promotion
            if (promotionType != chess::PieceType::NONE)
            {
                if (move.typeOf() != chess::Move::PROMOTION || move.promotionType() != promotionType)
                {
                    continue;
                }
            }

            // Found a match!
            return move;
        }
    }

    // No matching move found
    return chess::Move::NULL_MOVE;
}