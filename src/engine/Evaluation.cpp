#include "Evaluation.hpp"
#define whiteturn (board.sideToMove() == chess::Color::WHITE)
#define lighttile(sqi) (((sq >> 3) ^ sq) & 1)

void Evaluation::initPST()
{
    constexpr int PAWN_MID[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,              //
        98, 134, 61, 95, 68, 126, 34, -11,   //
        -6, 7, 26, 31, 65, 56, 25, -20,      //
        -14, 13, 6, 21, 23, 12, 17, -23,     //
        -27, -2, -5, 12, 17, 6, 10, -25,     //
        -26, -4, -4, -10, 3, 3, 33, -12,     //
        -35, -1, -20, -23, -15, 24, 38, -22, //
        0, 0, 0, 0, 0, 0, 0, 0,              //
    };
    constexpr int PAWN_END[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,                 //
        178, 173, 158, 134, 147, 132, 165, 187, //
        94, 100, 85, 67, 56, 53, 82, 84,        //
        32, 24, 13, 5, -2, 4, 17, 17,           //
        13, 9, -3, -7, -7, -8, 3, -1,           //
        4, 7, -6, 1, 0, -5, -1, -8,             //
        13, 8, 8, 10, 13, 0, 2, -7,             //
        0, 0, 0, 0, 0, 0, 0, 0,                 //
    };
    constexpr int KNIGHT_MID[64] = {
        -167, -89, -34, -49, 61, -97, -15, -107, //
        -73, -41, 72, 36, 23, 62, 7, -17,        //
        -47, 60, 37, 65, 84, 129, 73, 44,        //
        -9, 17, 19, 53, 37, 69, 18, 22,          //
        -13, 4, 16, 13, 28, 19, 21, -8,          //
        -23, -9, 12, 10, 19, 17, 25, -16,        //
        -29, -53, -12, -3, -1, 18, -14, -19,     //
        -105, -21, -58, -33, -17, -28, -19, -23, //
    };
    constexpr int KNIGHT_END[64] = {
        -58, -38, -13, -28, -31, -27, -63, -99, //
        -25, -8, -25, -2, -9, -25, -24, -52,    //
        -24, -20, 10, 9, -1, -9, -19, -41,      //
        -17, 3, 22, 22, 22, 11, 8, -18,         //
        -18, -6, 16, 25, 16, 17, 4, -18,        //
        -23, -3, -1, 15, 10, -3, -20, -22,      //
        -42, -20, -10, -5, -2, -20, -23, -44,   //
        -29, -51, -23, -15, -22, -18, -50, -64, //
    };
    constexpr int BISHOP_MID[64] = {
        -29, 4, -82, -37, -25, -42, 7, -8,     //
        -26, 16, -18, -13, 30, 59, 18, -47,    //
        -16, 37, 43, 40, 35, 50, 37, -2,       //
        -4, 5, 19, 50, 37, 37, 7, -2,          //
        -6, 13, 13, 26, 34, 12, 10, 4,         //
        0, 15, 15, 15, 14, 27, 18, 10,         //
        4, 15, 16, 0, 7, 21, 33, 1,            //
        -33, -3, -14, -21, -13, -12, -39, -21, //
    };
    constexpr int BISHOP_END[64] = {
        -14, -21, -11, -8, -7, -9, -17, -24, //
        -8, -4, 7, -12, -3, -13, -4, -14,    //
        2, -8, 0, -1, -2, 6, 0, 4,           //
        -3, 9, 12, 9, 14, 10, 3, 2,          //
        -6, 3, 13, 19, 7, 10, -3, -9,        //
        -12, -3, 8, 10, 13, 3, -7, -15,      //
        -14, -18, -7, -1, 4, -9, -15, -27,   //
        -23, -9, -23, -5, -9, -16, -5, -17,  //
    };
    constexpr int ROOK_MID[64] = {
        32, 42, 32, 51, 63, 9, 31, 43,      //
        27, 32, 58, 62, 80, 67, 26, 44,     //
        -5, 19, 26, 36, 17, 45, 61, 16,     //
        -24, -11, 7, 26, 24, 35, -8, -20,   //
        -36, -26, -12, -1, 9, -7, 6, -23,   //
        -45, -25, -16, -17, 3, 0, -5, -33,  //
        -44, -16, -20, -9, -1, 11, -6, -71, //
        -19, -13, 1, 17, 16, 7, -37, -26,   //
    };
    constexpr int ROOK_END[64] = {
        13, 10, 18, 15, 12, 12, 8, 5,    //
        11, 13, 13, 11, -3, 3, 8, 3,     //
        7, 7, 7, 5, 4, -3, -5, -3,       //
        4, 3, 13, 1, 2, 1, -1, 2,        //
        3, 5, 8, 4, -5, -6, -8, -11,     //
        -4, 0, -5, -1, -7, -12, -8, -16, //
        -6, -6, 0, 2, -9, -9, -11, -3,   //
        -9, 2, 3, -1, -5, -13, 4, -20,   //
    };
    constexpr int QUEEN_MID[64] = {
        -28, 0, 29, 12, 59, 44, 43, 45,      //
        -24, -39, -5, 1, -16, 57, 28, 54,    //
        -13, -17, 7, 8, 29, 56, 47, 57,      //
        -27, -27, -16, -16, -1, 17, -2, 1,   //
        -9, -26, -9, -10, -2, -4, 3, -3,     //
        -14, 2, -11, -2, -5, 2, 14, 5,       //
        -35, -8, 11, 2, 8, 15, -3, 1,        //
        -1, -18, -9, 10, -15, -25, -31, -50, //
    };
    constexpr int QUEEN_END[64] = {
        -9, 22, 22, 27, 27, 19, 10, 20,         //
        -17, 20, 32, 41, 58, 25, 30, 0,         //
        -20, 6, 9, 49, 47, 35, 19, 9,           //
        3, 22, 24, 45, 57, 40, 57, 36,          //
        -18, 28, 19, 47, 31, 34, 39, 23,        //
        -16, -27, 15, 6, 9, 17, 10, 5,          //
        -22, -23, -30, -16, -16, -23, -36, -32, //
        -33, -28, -22, -43, -5, -32, -20, -41,  //
    };
    constexpr int KING_MID[64] = {
        -65, 23, 16, -15, -56, -34, 2, 13,      //
        29, -1, -20, -7, -8, -4, -38, -29,      //
        -9, 24, 2, -16, -20, 6, 22, -22,        //
        -17, -20, -12, -27, -30, -25, -14, -36, //
        -49, -1, -27, -39, -46, -44, -33, -51,  //
        -14, -14, -22, -46, -44, -30, -15, -27, //
        1, 7, -8, -64, -43, -16, 9, 8,          //
        -15, 36, 12, -54, 8, -28, 24, 14,       //
    };
    constexpr int KING_END[64] = {
        -74, -35, -18, -18, -11, 15, 4, -17,    // A8, B8, ...
        -12, 17, 14, 17, 17, 38, 23, 11,        //
        10, 17, 23, 15, 20, 45, 44, 13,         //
        -8, 22, 24, 27, 26, 33, 26, 3,          //
        -18, -4, 21, 24, 27, 23, 9, -11,        //
        -19, -3, 11, 21, 23, 16, 7, -9,         //
        -27, -11, 4, 13, 14, 4, -5, -17,        //
        -53, -34, -21, -11, -28, -14, -24, -43, // A1, B1, ...
    };
    const int *PST_MID[6] = {
        PAWN_MID,
        KNIGHT_MID,
        BISHOP_MID,
        ROOK_MID,
        QUEEN_MID,
        KING_MID,
    };
    const int *PST_END[6] = {
        PAWN_END,
        KNIGHT_END,
        BISHOP_END,
        ROOK_END,
        QUEEN_END,
        KING_END,
    };

    // Fill the PST array
    for (int p = 0; p < 6; p++)
    {
        for (int sq = 0; sq < 64; sq++)
        {
            // flip to put sq56 -> A1 and so on
            PST[p][sq][0] = PST_MID[p][sq ^ 56];
            PST[p][sq][1] = PST_END[p][sq ^ 56];
            // flip for black
            PST[p + 6][sq][0] = -PST_MID[p][sq];
            PST[p + 6][sq][1] = -PST_END[p][sq];
        }
    }
}

void Evaluation::initPawnmask()
{
    // Initialize file masks
    for (int file = 0; file < 8; file++)
    {
        fileMasks[file] = 0;
        for (int rank = 0; rank < 8; rank++)
        {
            fileMasks[file] |= (1ULL << (rank * 8 + file));
        }
    }
}

chess::Bitboard Evaluation::getWhitePassedMask(int sq) const
{
    int file = sq % 8;
    int rank = sq / 8;
    chess::Bitboard result = 0;

    // Add files (current and adjacent)
    for (int f = std::max(0, file - 1); f <= std::min(7, file + 1); f++)
    {
        // Add squares in front of the pawn
        for (int r = rank + 1; r < 8; r++)
        {
            result |= (1ULL << (r * 8 + f));
        }
    }

    return result;
}

chess::Bitboard Evaluation::getBlackPassedMask(int sq) const
{
    int file = sq % 8;
    int rank = sq / 8;
    chess::Bitboard result = 0;

    // Add files (current and adjacent)
    for (int f = std::max(0, file - 1); f <= std::min(7, file + 1); f++)
    {
        // Add squares behind the pawn
        for (int r = rank - 1; r >= 0; r--)
        {
            result |= (1ULL << (r * 8 + f));
        }
    }

    return result;
}

chess::Bitboard Evaluation::getIsolatedMask(int sq) const
{
    int file = sq % 8;
    chess::Bitboard result = 0;

    // Add adjacent files
    if (file > 0)
        result |= fileMasks[file - 1];
    if (file < 7)
        result |= fileMasks[file + 1];

    return result;
}

int Evaluation::evaluate(const chess::Board &board) const
{
    int eval_mid = 0, eval_end = 0;
    int phase = 0;
    auto pieces = board.occ();
    // draw evaluation
    int wbish_on_w = 0, wbish_on_b = 0; // number of white bishop on light and dark tiles
    int bbish_on_w = 0, bbish_on_b = 0; // number of black bishop on light and dark tiles
    int wbish = 0, bbish = 0;
    int wknight = 0, bknight = 0;
    bool minor_only = true;
    // mobility
    int wkr = 0, bkr = 0;         // king rank
    int wkf = 0, bkf = 0;         // king file
    int bishmob = 0, rookmob = 0; // number of squares bishop and rooks see (white - black)
    // xray bitboards
    auto wbishx = pieces & ~board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE);
    auto bbishx = pieces & ~board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK);
    auto wrookx = wbishx & ~board.pieces(chess::PieceType::ROOK, chess::Color::WHITE);
    auto brookx = bbishx & ~board.pieces(chess::PieceType::ROOK, chess::Color::BLACK);
    auto wpawns = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE);
    auto bpawns = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK);
    // loop through all pieces
    while (pieces)
    {
        auto sq = chess::builtin::poplsb(pieces);
        int sqi = (int)sq;
        int piece = (int)board.at(sq);
        // add material value
        eval_mid += PVAL[piece][0];
        eval_end += PVAL[piece][1];
        // add positional value
        eval_mid += PST[piece][sqi][0];
        eval_end += PST[piece][sqi][1];
        switch (piece)
        {
        // pawn structure
        case 0:
            minor_only = false;
            // passed (+ for white)
            if ((getWhitePassedMask(sqi) & bpawns) == 0)
            {
                eval_mid += PAWN_PASSED_WEIGHT[7 - (sqi / 8)][0];
                eval_end += PAWN_PASSED_WEIGHT[7 - (sqi / 8)][1];
            }
            // isolated (- for white)
            if ((getIsolatedMask(sqi) & wpawns) == 0)
            {
                eval_mid -= PAWN_ISOLATION_WEIGHT[0];
                eval_end -= PAWN_ISOLATION_WEIGHT[1];
            }
            break;
        case 6:
            minor_only = false;
            // passed (- for white)
            if ((getBlackPassedMask(sqi) & wpawns) == 0)
            {
                eval_mid -= PAWN_PASSED_WEIGHT[sqi / 8][0];
                eval_end -= PAWN_PASSED_WEIGHT[sqi / 8][1];
            }
            // isolated (+ for white)
            if ((getIsolatedMask(sqi) & bpawns) == 0)
            {
                eval_mid += PAWN_ISOLATION_WEIGHT[0];
                eval_end += PAWN_ISOLATION_WEIGHT[1];
            }
            break;
        // knight count
        case 1:
            phase++;
            wknight++;
            break;
        case 7:
            phase++;
            bknight++;
            break;
        // bishop mobility (xrays queens)
        case 2:
            phase++;
            wbish++;
            wbish_on_w += lighttile(sqi);
            wbish_on_b += !lighttile(sqi);
            bishmob += chess::builtin::popcount(chess::attacks::bishop(sq, wbishx));
            break;
        case 8:
            phase++;
            bbish++;
            bbish_on_w += lighttile(sqi);
            bbish_on_b += !lighttile(sqi);
            bishmob -= chess::builtin::popcount(chess::attacks::bishop(sq, bbishx));
            break;
        // rook mobility (xrays rooks and queens)
        case 3:
            phase += 2;
            minor_only = false;
            rookmob += chess::builtin::popcount(chess::attacks::rook(sq, wrookx));
            break;
        case 9:
            phase += 2;
            minor_only = false;
            rookmob -= chess::builtin::popcount(chess::attacks::rook(sq, brookx));
            break;
        // queen count
        case 4:
            phase += 4;
            minor_only = false;
            break;
        case 10:
            phase += 4;
            minor_only = false;
            break;
        // king proximity
        case 5:
            wkr = (int)chess::utils::squareRank(sq);
            wkf = (int)chess::utils::squareFile(sq);
            break;
        case 11:
            bkr = (int)chess::utils::squareRank(sq);
            bkf = (int)chess::utils::squareFile(sq);
            break;
        }
    }
    // mobility
    eval_mid += bishmob * MOBILITY_BISHOP[0];
    eval_end += bishmob * MOBILITY_BISHOP[1];
    eval_mid += rookmob * MOBILITY_ROOK[0];
    eval_end += rookmob * MOBILITY_ROOK[1];
    // bishop pair
    bool wbish_pair = wbish_on_w && wbish_on_b;
    bool bbish_pair = bbish_on_w && bbish_on_b;
    if (wbish_pair)
    {
        eval_mid += BISH_PAIR_WEIGHT[0];
        eval_end += BISH_PAIR_WEIGHT[1];
    }
    if (bbish_pair)
    {
        eval_mid -= BISH_PAIR_WEIGHT[0];
        eval_end -= BISH_PAIR_WEIGHT[1];
    }
    // convert perspective
    if (!whiteturn)
    {
        eval_mid *= -1;
        eval_end *= -1;
    }
    // King proximity bonus (if winning)
    int king_dist = abs(wkr - bkr) + abs(wkf - bkf);
    if (eval_mid >= 0)
        eval_mid += KING_DIST_WEIGHT[0] * (14 - king_dist);
    if (eval_end >= 0)
        eval_end += KING_DIST_WEIGHT[1] * (14 - king_dist);
    // Bishop corner (if winning)
    int ourbish_on_w = (whiteturn) ? wbish_on_w : bbish_on_w;
    int ourbish_on_b = (whiteturn) ? wbish_on_b : bbish_on_b;
    int ekr = (whiteturn) ? bkr : wkr;
    int ekf = (whiteturn) ? bkf : wkf;
    int wtile_dist = std::min(ekf + (7 - ekr), (7 - ekf) + ekr); // to A8 and H1
    int btile_dist = std::min(ekf + ekr, (7 - ekf) + (7 - ekr)); // to A1 and H8
    if (eval_mid >= 0)
    {
        if (ourbish_on_w)
            eval_mid += BISH_CORNER_WEIGHT[0] * (7 - wtile_dist);
        if (ourbish_on_b)
            eval_mid += BISH_CORNER_WEIGHT[0] * (7 - btile_dist);
    }
    if (eval_end >= 0)
    {
        if (ourbish_on_w)
            eval_end += BISH_CORNER_WEIGHT[1] * (7 - wtile_dist);
        if (ourbish_on_b)
            eval_end += BISH_CORNER_WEIGHT[1] * (7 - btile_dist);
    }
    // apply phase
    int eg_weight = 256 * std::max(0, 24 - phase) / 24;
    int eval = ((256 - eg_weight) * eval_mid + eg_weight * eval_end) / 256;
    // draw division
    int wminor = wbish + wknight;
    int bminor = bbish + bknight;
    if (minor_only && wminor <= 2 && bminor <= 2)
    {
        if ((wminor == 1 && bminor == 1) ||                                     // 1 vs 1
            ((wbish + bbish == 3) && (wminor + bminor == 3)) ||                 // 2B vs B
            ((wknight == 2 && bminor <= 1) || (bknight == 2 && wminor <= 1)) || // 2N vs 0:1
            (!wbish_pair && wminor == 2 && bminor == 1) ||                      // 2 vs 1, not bishop pair
            (!bbish_pair && bminor == 2 && wminor == 1))
            return eval / DRAW_DIVIDE_SCALE;
    }
    return eval;
}