#include "ChessEngine.hpp"
#include "See.hpp"
#include <iomanip>

ChessEngine::ChessEngine()
    : rng(std::random_device{}()), tt(64), history(), killers()
{
    initializeOpeningBook();
}

bool ChessEngine::initializeOpeningBook()
{
    // Use only Adams.pgn for opening book
    std::string path = "assets/opening/Adams.pgn";
    std::cout << "Initializing opening book!!!" << std::endl;
    return openingBook.initializeFromFile(path);
}

void ChessEngine::setMaxBookMoves(int maxMoves)
{
    openingBook.setMaxBookMoves(maxMoves);
}

chess::Move ChessEngine::getBestMove(chess::Board &board)
{
    if (useOpeningBook)
    {
        chess::Move bookMove = openingBook.getBookMove(board);
        if (bookMove != chess::Move::NULL_MOVE)
        {
            std::cout << "Using opening book move: " << bookMove << std::endl;
            moveCounter++;
            return bookMove;
        }
    }

    history.clear();
    auto startTime = std::chrono::steady_clock::now();

    SearchStats stats;
    stats.reset();

    chess::Move bestMove = chess::Move::NULL_MOVE;

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    if (moves.size() == 1)
    {
        moveCounter++;
        return moves[0];
    }

    if (moves.empty())
    {
        return chess::Move::NULL_MOVE;
    }

    // Perform iterative deepening
    for (int depth = 1; depth <= MAX_DEPTH; depth++)
    {
        stats.depth = depth;
        stats.reset();

        int alpha = -32000;
        int beta = 32000;

        uint64_t nodes = 0;
        int score;

        score = negamax(board, depth, 0, alpha, beta, nodes);

        stats.score = score;
        stats.nodes = nodes;

        // Track best move found at each depth
        for (const auto &move : moves)
        {
            board.makeMove(move);
            int moveScore = -negamax(board, depth - 1, 1, -beta, -alpha, nodes);
            board.unmakeMove(move);

            if (moveScore > alpha)
            {
                alpha = moveScore;
                bestMove = move;
                stats.bestMove = bestMove;
            }
        }

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime);
        stats.duration = elapsed;

        printSearchInfo(stats);

        TTStats ttStats = tt.get_stats();
        std::cout << "TT Stats - Depth " << depth << ": "
                  << "Size: " << ttStats.size << "/" << ttStats.capacity
                  << ", Usage: " << std::fixed << std::setprecision(2) << ttStats.usage << "%"
                  << ", Hit Rate: " << ttStats.hit_rate << "%"
                  << ", Collisions: " << ttStats.collisions
                  << std::endl;

        if (elapsed.count() > TIME_LIMIT * 1000 / 2) 
        {
            break;
        }
    }

    if (bestMove == chess::Move::NULL_MOVE && !moves.empty())
    {
        std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
        bestMove = moves[dist(rng)];
    }

    auto endTime = std::chrono::steady_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "\nSearch completed in " << totalTime << "ms" << std::endl;
    std::cout << "Best move: " << chess::uci::moveToUci(bestMove) << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;

    moveCounter++;
    return bestMove;
}

int ChessEngine::negamax(chess::Board &board, int depth, int ply, int alpha, int beta, uint64_t &nodes)
{
    nodes++; 

    if (alpha < -CHECKMATE_SCORE + ply)
        alpha = -CHECKMATE_SCORE + ply;
    if (beta > CHECKMATE_SCORE - ply)
        beta = CHECKMATE_SCORE - ply;
    if (alpha >= beta)
        return alpha;

    if (board.isInsufficientMaterial() || board.isRepetition(2) || board.isHalfMoveDraw())
    {
        return DRAW_SCORE; 
    }

    if (depth <= 0)
    {
        return quiesence(board, alpha, beta, nodes, ply);
    }

    uint64_t hashKey = board.hash();
    auto [found, score] = tt.lookup(hashKey, depth, alpha, beta);
    if (found)
    {
        return score;
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // Check for checkmate/stalemate
    if (moves.empty())
    {
        if (board.inCheck())
        {
            return -CHECKMATE_SCORE + ply;
        }
        else
        {
            return DRAW_SCORE;
        }
    }

    orderMoves(board, moves, ply);

    int bestScore = -INF;
    int alphaOriginal = alpha;
    chess::Move bestMove = chess::Move::NULL_MOVE;

    for (int i = 0; i < moves.size(); i++)
    {
        chess::Move move = moves[i];

        bool isReduced = false;
        bool isCapture = board.at(move.to()) != chess::Piece::NONE;
        bool isPromotion = move.typeOf() == chess::Move::PROMOTION;
        bool givesCheck = false;

        board.makeMove(move);

        givesCheck = board.inCheck();

        int newDepth = depth - 1;

        if (depth >= 3 && i >= 4 && !isCapture && !isPromotion && !givesCheck)
        {
            isReduced = true;
            newDepth = depth - 2;
        }

        int score;

        if (i == 0) {
            score = -negamax(board, newDepth, ply + 1, -beta, -alpha, nodes);
        } else {
            score = -negamax(board, newDepth, ply + 1, -alpha - 1, -alpha, nodes);
                
            if (isReduced && score > alpha) {

                score = -negamax(board, depth - 1, ply + 1, -alpha - 1, -alpha, nodes);
            }
            
            if (score > alpha && score < beta) {
                score = -negamax(board, depth - 1, ply + 1, -beta, -alpha, nodes);
            }
        }

        board.unmakeMove(move);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha)
        {
            alpha = score;

            if (alpha >= beta)
            {
                if (!board.isCapture(move)) {
                    killers.put(move, ply);
                    history.update(move, depth, board.sideToMove() == chess::Color::WHITE);
                }
                tt.store(hashKey, beta, TTFlag::LOWER_BOUND, depth);
                return beta; 
            }
        }
    }

    TTFlag flag = alpha > alphaOriginal ? TTFlag::EXACT_SCORE : TTFlag::UPPER_BOUND;
    tt.store(hashKey, bestScore, flag, depth);

    return bestScore;
}

int ChessEngine::quiesence(chess::Board &board, int alpha, int beta, uint64_t &nodes, int ply)
{
    nodes++; 
    
    if (alpha < -CHECKMATE_SCORE + ply) alpha = -CHECKMATE_SCORE + ply;
    if (beta > CHECKMATE_SCORE - ply) beta = CHECKMATE_SCORE - ply;
    if (alpha >= beta)
        return alpha;
        
    if (board.isInsufficientMaterial() || board.isRepetition(1) || board.isHalfMoveDraw())
    {
        return DRAW_SCORE; 
    }
    
    const int MAX_QUIESCENCE_DEPTH = 10;
    if (ply >= MAX_QUIESCENCE_DEPTH)
        return evaluatePosition(board);
        
    bool inCheck = board.inCheck();
    uint64_t hashKey = board.hash();
    auto [hit, score] = tt.lookup(hashKey, 0, alpha, beta);
    
    if (hit)
    {
        return score;
    }

    int standPat = -INF;
    if (!inCheck)
    {
        standPat = evaluatePosition(board);
        if (standPat >= beta)
        {
            tt.store(hashKey, beta, TTFlag::LOWER_BOUND, 0);
            return beta;
        }
        if (standPat > alpha)
        {
            alpha = standPat;
        }
    }

    chess::Movelist moves;

    if (inCheck)
    {
        chess::movegen::legalmoves(moves, board);
        orderMoves(board, moves, ply);

    }
    else
    {
        chess::movegen::legalmoves<chess::MoveGenType::CAPTURE>(moves, board);
        orderMoves(board, moves, ply);
    }

    for (const auto &move : moves)
    {
        if (!inCheck)
        {
            int moveGain = move.score();

            if (standPat + moveGain + DELTA <= alpha)
            {
                continue;
            }
        }

        if (!inCheck && !SEE::isGoodCapture(move, board, -20))
        {
            continue;
        }

        board.makeMove(move);

        int score = -quiesence(board, -beta, -alpha, nodes, ply + 1);

        board.unmakeMove(move);

        if (score >= beta)
        {
            tt.store(hashKey, beta, TTFlag::LOWER_BOUND, 0);
            return beta;
        }
        if (score > alpha)
            alpha = score;
    }
    
    if (inCheck && moves.size() == 0)
    {
        return -CHECKMATE_SCORE + ply; 
    }

    tt.store(hashKey, alpha, TTFlag::UPPER_BOUND, 0);
    return alpha;
}

void ChessEngine::orderMoves(chess::Board &board, chess::Movelist &moves, const int ply)
{
    for (auto &move : moves)
    {
        scoreMoves(board, move, ply);
    }

    moves.sort();
}

void ChessEngine::scoreMoves(const chess::Board &board, chess::Move &move, const int ply)
{
    int score = 0;

    if (board.isCapture(move))
    {
        if (move.typeOf() == chess::Move::ENPASSANT) {
            score = SEE::getMvvLvaScore(chess::PieceType::PAWN,
                                    chess::PieceType::PAWN)  + 1000;
        } else {
            chess::PieceType captured = chess::utils::typeOfPiece(board.at(move.to()));
            chess::PieceType attacker = chess::utils::typeOfPiece(board.at(move.from()));

            score = SEE::getMvvLvaScore(captured, attacker);

            if (score < 6000)
            {
                if (SEE::isGoodCapture(move, board, 0))
                {
                    score += GOOD_CAPTURE_WEIGHT;
                }
                else
                {
                    score = 0;
                }
            }
        }

    }
    else
    {
        if (ply > 0 && killers.isKiller(move, ply)) {
            score += 50;
        }
        score += history.get(move, board.sideToMove() == chess::Color::WHITE);    }

    if (move.typeOf() == chess::Move::PROMOTION)
    {
        if (move.promotionType() == chess::PieceType::QUEEN)
            score += 100000;
        else if (move.promotionType() == chess::PieceType::ROOK)
            score += 500;
        else if (move.promotionType() == chess::PieceType::BISHOP ||
                 move.promotionType() == chess::PieceType::KNIGHT)
            score += 300;
    }

    move.setScore(score);
}

int ChessEngine::evaluatePosition(const chess::Board &board)
{
    int score = evaluation.evaluate(board);
    return score;
}

void ChessEngine::printSearchInfo(const SearchStats &stats)
{
    auto timeInMs = stats.duration.count();
    uint64_t nps = timeInMs > 0 ? (stats.nodes * 1000) / timeInMs : 0;

    std::cout << "Depth: " << stats.depth
              << ", Score: " << stats.score
              << ", Nodes: " << stats.nodes
              << ", Time: " << timeInMs
              << ", NPS: " << nps
              << ", Best Move: " << stats.bestMove
              << std::endl;
}