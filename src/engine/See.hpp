//
// Created by ADMIN on 4/27/2025.
//

#ifndef SEE_HPP
#define SEE_HPP
#include "../chess.hpp"

namespace SEE {
  const int PIECE_VALUES[6] = {100, 320, 330, 500, 900, 10000};

  inline int getPieceValue(const chess::Square sq, const chess::Board& board) {
    return PIECE_VALUES[static_cast<int>(board.at<chess::PieceType>(sq))];
  }

  inline chess::Square getLLASquare(chess::Bitboard attackers,
                                    const chess::Board& board) {
    for (int pt=0; pt<6; pt++) {
      auto specificTypeAttackers = attackers &
                                          board.pieces((chess::PieceType)pt);
      if (specificTypeAttackers)
        return chess::builtin::poplsb(specificTypeAttackers);
    }
    return chess::NO_SQ;
  }

  inline int staticExchangeEvaluate(chess::Move &move, chess::Board &board) {
    chess::Square exchangeSquare = move.to();
    chess::Square nextVictim = move.from();
    auto removedCaptureOcc = board.occ() ^ (1ULL<<nextVictim);
    chess::Color oppColor = ~board.sideToMove();
    int gain[26] = {0};
    int numCaptures = 0;

    // Check if the move is an en passant capture
    if (move.typeOf() == chess::Move::ENPASSANT) {
      // Assign the value of a pawn to the gain array
      gain[0] = PIECE_VALUES[0];

      // Determine the square of the captured pawn during en passant
      chess::Square enpassantSquare = (board.sideToMove() == chess::Color::WHITE)
                                      ? exchangeSquare + chess::Direction::SOUTH
                                      : exchangeSquare + chess::Direction::NORTH;

      // Update the occupancy bitboard to reflect the removal of the captured pawn
      removedCaptureOcc ^= (1ULL << enpassantSquare);
    } else {
      // For regular captures, assign the value of the captured piece to the gain array
      gain[0] = getPieceValue(exchangeSquare, board);
    }

    //List of direct attackers
    chess::Bitboard queens = board.pieces(chess::PieceType::QUEEN);
    chess::Bitboard rooks = board.pieces(chess::PieceType::ROOK) | queens;
    chess::Bitboard bishops = board.pieces(chess::PieceType::BISHOP) | queens;
    auto allAttackers =
      (chess::attacks::pawn(oppColor, exchangeSquare)
        & board.pieces(chess::PieceType::PAWN, ~oppColor));
    allAttackers |=
      (chess::attacks::pawn(~oppColor, exchangeSquare)
        & board.pieces(chess::PieceType::PAWN, oppColor));
    allAttackers |=
      (chess::attacks::knight(exchangeSquare)
        & board.pieces(chess::PieceType::KNIGHT));
    allAttackers |=
      (chess::attacks::bishop(exchangeSquare, removedCaptureOcc)
        & bishops);
    allAttackers |=
      (chess::attacks::rook(exchangeSquare, removedCaptureOcc)
        & rooks);
    allAttackers |=
      (chess::attacks::king(exchangeSquare)
        & board.pieces(chess::PieceType::KING));
    while (true) {
      numCaptures++;
      gain[numCaptures] = getPieceValue(nextVictim, board)
                          - gain[numCaptures - 1];
      allAttackers &= removedCaptureOcc;
      auto attackers = allAttackers & board.us(oppColor);
      if (!attackers) {
        break;
      }

      oppColor = ~oppColor;
      nextVictim = getLLASquare(attackers, board);
      removedCaptureOcc ^= (1ULL << nextVictim);
      allAttackers |= chess::attacks::bishop(exchangeSquare, removedCaptureOcc)
                      & bishops;
      allAttackers |= chess::attacks::rook(exchangeSquare, removedCaptureOcc)
                      & rooks;
    }

    while (--numCaptures >= 0) {
      gain[numCaptures-1] = -std::max(-gain[numCaptures-1], gain[numCaptures]);
    }

    return gain[0];
  }

  inline bool isGoodCapture(const chess::Move &move, const chess::Board &board,
                            const int threshold) {
    chess::Square exchangeSquare = move.to();
    chess::Square nextVictim = move.from();
    auto removedCaptureOcc = board.occ() ^ (1ULL<<nextVictim);
    chess::Color oppColor = ~board.sideToMove();
    int gain = 0;

    // Check if the move is an en passant capture
    if (move.typeOf() == chess::Move::ENPASSANT) {
      // Assign the value of a pawn to the gain array
      gain = PIECE_VALUES[0] - threshold;

      // Determine the square of the captured pawn during en passant
      chess::Square enpassantSquare = (board.sideToMove() == chess::Color::WHITE)
                                      ? exchangeSquare + chess::Direction::SOUTH
                                      : exchangeSquare + chess::Direction::NORTH;

      // Update the occupancy bitboard to reflect the removal of the captured pawn
      removedCaptureOcc ^= (1ULL << enpassantSquare);
    } else {
      // For regular captures, assign the value of the captured piece to the gain array
      gain = getPieceValue(exchangeSquare, board) - threshold;
    }
    if (gain < 0) return false;

    gain -= getPieceValue(move.from(), board);
    if (gain > 0) return true;

    //List of direct attackers
    chess::Bitboard queens = board.pieces(chess::PieceType::QUEEN);
    chess::Bitboard rooks = board.pieces(chess::PieceType::ROOK) | queens;
    chess::Bitboard bishops = board.pieces(chess::PieceType::BISHOP) | queens;
    auto allAttackers =
      (chess::attacks::pawn(oppColor, exchangeSquare)
        & board.pieces(chess::PieceType::PAWN, ~oppColor));
    allAttackers |=
      (chess::attacks::pawn(~oppColor, exchangeSquare)
        & board.pieces(chess::PieceType::PAWN, oppColor));
    allAttackers |=
      (chess::attacks::knight(exchangeSquare)
        & board.pieces(chess::PieceType::KNIGHT));
    allAttackers |=
      (chess::attacks::bishop(exchangeSquare, removedCaptureOcc)
        & bishops);
    allAttackers |=
      (chess::attacks::rook(exchangeSquare, removedCaptureOcc)
        & rooks);
    allAttackers |=
      (chess::attacks::king(exchangeSquare)
        & board.pieces(chess::PieceType::KING));
    while (true) {
      allAttackers &= removedCaptureOcc;
      auto attackers = allAttackers & board.us(oppColor);
      if (!attackers) {
        break;
      }

      oppColor = ~oppColor;
      nextVictim = getLLASquare(attackers, board);
      gain = -gain - 1 -  getPieceValue(nextVictim, board);
      if (gain >= 0) {
        if (board.at<chess::PieceType>(nextVictim)==chess::PieceType::KING &&
                   attackers&board.us(oppColor))
          oppColor = ~oppColor;
        break;
      }

      removedCaptureOcc ^= (1ULL << nextVictim);
      allAttackers |= chess::attacks::bishop(exchangeSquare, removedCaptureOcc)
                      & bishops;
      allAttackers |= chess::attacks::rook(exchangeSquare, removedCaptureOcc)
                      & rooks;
    }

    return oppColor != board.sideToMove();
  }


}

#endif //SEE_HPP
