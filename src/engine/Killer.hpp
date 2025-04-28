//
// Created by ADMIN on 4/28/2025.
//

#ifndef KILLER_HPP
#define KILLER_HPP
#include "../chess.hpp"

namespace KILLER {
class Killers {
private:
  std::vector<chess::Move> _killers;
  static constexpr int MAX_DEPTH = 7;


public:
  Killers(): _killers(MAX_DEPTH*2, chess::Move::NO_MOVE) {}



  void put(const chess::Move move, const int ply) {
    if (move != _killers[2*ply]) {
      _killers[2*ply+1] = _killers[2*ply];
      _killers[2*ply] = move;
    }
  }


  bool isKiller(const chess::Move move, const int ply) const {
    return (move==_killers[2*ply] || move==_killers[2*ply+1]);
  }


  // Clears the killer storage
  void clear() {
    std::fill(_killers.begin(), _killers.end(),
              chess::Move::NO_MOVE);
  }
};
}


#endif //KILLER_HPP