//
// Created by ADMIN on 4/28/2025.
//

#ifndef HISTORY_HPP
#define HISTORY_HPP
#include "../chess.hpp"
#include <cstring>

namespace HISTORY {
class History {

private:
  int _history[2][64][64];

public:
  History() {
    clear();
  }

  void update(const chess::Move move, const int depth, const int side) {
    const int fromSquare = (int)move.from();
    const int toSquare = (int)move.to();
    _history[side][fromSquare][toSquare] += depth * depth;
  }


  int get(const chess::Move move, const int side) const {
    int fromSquare = (int)move.from();
    int toSquare = (int)move.to();
    return _history[side][fromSquare][toSquare];
  }



  void clear() {
    std::memset(_history, 0, sizeof(_history));
  }
};
}

#endif //HISTORY_HPP
