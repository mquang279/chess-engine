#ifndef OPENING_MOVE_HPP
#define OPENING_MOVE_HPP

#include "../chess.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

class OpeningMove
{
public:
    OpeningMove();
    ~OpeningMove() = default;

    // Initialize opening book from PGN files in assets/opening directory
    bool initialize(const std::string &openingDirPath = "assets/opening");

    // Initialize opening book from a specific PGN file
    bool initializeFromFile(const std::string &pgnFilePath);

    chess::Move getBookMove(const chess::Board &board);

    void setMaxBookMoves(int max) { maxBookMoves = max; }

    int getMaxBookMoves() const { return maxBookMoves; }

private:
    struct OpeningNode
    {
        std::unordered_map<uint64_t, std::vector<std::pair<chess::Move, int>>> positions;
    };

    std::unique_ptr<OpeningNode> openingBook;
    int maxBookMoves = 12; // Default to 12 moves from opening book

    // Parse a PGN file and add its moves to the opening book
    bool parsePgnFile(const std::string &filepath);

    // Extract moves from a PGN game and add them to the opening book
    void addGameToBook(const std::string &moveText);

    // Converts algebraic notation (e.g. "e4") to a chess::Move
    chess::Move algebraicToMove(const chess::Board &board, const std::string &moveStr);
};

#endif // OPENING_MOVE_HPP