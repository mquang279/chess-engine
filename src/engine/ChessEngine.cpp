#include "ChessEngine.hpp"
#include "See.hpp"
#include <iomanip>

ChessEngine::ChessEngine()
    : rng(std::random_device{}()), tt(64)
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

    // Start timer
    auto startTime = std::chrono::steady_clock::now();

    // Initialize search statistics
    SearchStats stats;
    stats.reset();

    // Initialize best move to invalid
    chess::Move bestMove = chess::Move::NULL_MOVE;

    // Generate legal moves
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // If only one move, return immediately
    if (moves.size() == 1)
    {
        moveCounter++;
        return moves[0];
    }

    // Handle special case with no legal moves
    if (moves.empty())
    {
        return chess::Move::NULL_MOVE;
    }

    // Perform iterative deepening
    for (int depth = 1; depth <= MAX_DEPTH; depth++)
    {
        stats.depth = depth;
        stats.reset();

        // Set initial alpha-beta bounds - full window for every depth
        int alpha = -32000;
        int beta = 32000;

        // Perform negamax search
        uint64_t nodes = 0;
        int score;

        // Main search call with full window - start with ply=0
        score = negamax(board, depth, 0, alpha, beta, nodes);

        // Store statistics
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

        // Check elapsed time
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime);
        stats.duration = elapsed;

        // Print search information
        printSearchInfo(stats);

        // Print TT stats
        TTStats ttStats = tt.get_stats();
        std::cout << "TT Stats - Depth " << depth << ": "
                  << "Size: " << ttStats.size << "/" << ttStats.capacity
                  << ", Usage: " << std::fixed << std::setprecision(2) << ttStats.usage << "%"
                  << ", Hit Rate: " << ttStats.hit_rate << "%"
                  << ", Collisions: " << ttStats.collisions
                  << std::endl;

        // Check if we've exceeded the time limit
        if (elapsed.count() > TIME_LIMIT * 1000 / 2) // Use half the available time for safety
        {
            break;
        }
    }

    // If still no valid move found (unlikely), pick a random legal move
    if (bestMove == chess::Move::NULL_MOVE && !moves.empty())
    {
        std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
        bestMove = moves[dist(rng)];
    }

    // Calculate total time taken
    auto endTime = std::chrono::steady_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Print total time and best move chosen
    std::cout << "\nSearch completed in " << totalTime << "ms" << std::endl;
    std::cout << "Best move: " << chess::uci::moveToUci(bestMove) << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;

    moveCounter++;
    return bestMove;
}

int ChessEngine::negamax(chess::Board &board, int depth, int ply, int alpha, int beta, uint64_t &nodes)
{
    nodes++; // Increment node counter

    // Mate distance pruning - revised implementation
    if (alpha < -CHECKMATE_SCORE + ply)
        alpha = -CHECKMATE_SCORE + ply;
    if (beta > CHECKMATE_SCORE - ply)
        beta = CHECKMATE_SCORE - ply;
    if (alpha >= beta)
        return alpha;

    // Check for immediate draw conditions
    if (board.isInsufficientMaterial() || board.isRepetition(2) || board.isHalfMoveDraw())
    {
        return DRAW_SCORE; // Draw
    }

    // Base case: leaf node (evaluate position or use quiescence search)
    if (depth <= 0)
    {
        return quiesence(board, alpha, beta, nodes, ply);
    }

    // Transposition table lookup
    uint64_t hashKey = board.hash();
    auto [found, score] = tt.lookup(hashKey, depth, alpha, beta);
    if (found)
    {
        return score;
    }

    // Generate legal moves
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    // Check for checkmate/stalemate
    if (moves.empty())
    {
        if (board.inCheck())
        {
            // Checkmate - make closer mates have a higher score
            return -CHECKMATE_SCORE + ply;
        }
        else
        {
            return DRAW_SCORE; // Stalemate
        }
    }

    // Order moves for better pruning
    orderMoves(board, moves);

    int bestScore = -INF;
    int alphaOriginal = alpha;
    chess::Move bestMove = chess::Move::NULL_MOVE;

    // Iterate through each move
    for (int i = 0; i < moves.size(); i++)
    {
        chess::Move move = moves[i];

        // Late Move Reduction
        bool isReduced = false;
        bool isCapture = board.at(move.to()) != chess::Piece::NONE;
        bool isPromotion = move.typeOf() == chess::Move::PROMOTION;
        bool givesCheck = false;

        // Make the move
        board.makeMove(move);

        // Check if this move gives check
        givesCheck = board.inCheck();

        // Late move reduction for quiet moves after we've searched several moves
        int newDepth = depth - 1;

        // Perform late move reduction for quiet moves after first few moves
        if (depth >= 3 && i >= 4 && !isCapture && !isPromotion && !givesCheck)
        {
            isReduced = true;
            newDepth = depth - 2; // Reduce search depth
        }

        // Recursive negamax call with negated bounds
        int score;

        // Principal Variation Search implementation
        if (i == 0) {
            // Search first move with full window
            score = -negamax(board, newDepth, ply + 1, -beta, -alpha, nodes);
        } else {
            // Try a null-window search for remaining move
            // Reduced-depth null-window search
            score = -negamax(board, newDepth, ply + 1, -alpha - 1, -alpha, nodes);
                
            // If the reduced search exceeds alpha, we need to re-search at full depth
            if (isReduced && score > alpha) {

                score = -negamax(board, depth - 1, ply + 1, -alpha - 1, -alpha, nodes);
            }
            
            // If the null-window search failed high but didn't exceed beta, 
            // we need a full-window search
            if (score > alpha && score < beta) {
                score = -negamax(board, depth - 1, ply + 1, -beta, -alpha, nodes);
            }
        }

        // Undo the move
        board.unmakeMove(move);

        // Update best score
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        // Alpha-beta pruning
        if (score > alpha)
        {
            alpha = score;

            // If we found a move that's too good, no need to search further
            if (alpha >= beta)
            {
                tt.store(hashKey, beta, TTFlag::LOWER_BOUND, depth);
                return beta; // Beta cutoff (fail-high)
            }
        }
    }

    // Store result in transposition table
    TTFlag flag = alpha > alphaOriginal ? TTFlag::EXACT_SCORE : TTFlag::UPPER_BOUND;
    tt.store(hashKey, bestScore, flag, depth);

    return bestScore;
}

int ChessEngine::quiesence(chess::Board &board, int alpha, int beta, uint64_t &nodes, int ply)
{
    nodes++; // Increment node counter
    
    // Mate distance pruning - revised implementation
    if (alpha < -CHECKMATE_SCORE + ply) alpha = -CHECKMATE_SCORE + ply;
    if (beta > CHECKMATE_SCORE - ply) beta = CHECKMATE_SCORE - ply;
    if (alpha >= beta)
        return alpha;
        
    // Quick check for draw conditions
    if (board.isInsufficientMaterial() || board.isRepetition(1) || board.isHalfMoveDraw())
    {
        return DRAW_SCORE; // Draw
    }
    
    // Maximum quiescence search depth safety check
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

    if (!inCheck)
    {
        int standPat = evaluatePosition(board);
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
    // In check, we must consider all legal moves
    if (inCheck)
    {
        chess::movegen::legalmoves(moves, board);
    }
    else
    {
        // Only consider captures and promotions in quiescence search
        chess::movegen::legalmoves<chess::MoveGenType::CAPTURE>(moves, board);
        orderMoves(board, moves);
    }

    for (const auto &move : moves)
    {
        // Static Exchange Evaluation (SEE) pruning for bad captures
        if (!inCheck && !SEE::isGoodCapture(move, board, -20))
        {
            continue; // Skip capturing moves that lose material
        }

        board.makeMove(move);

        // Recursive quiescence search with incremented ply
        int score = -quiesence(board, -beta, -alpha, nodes, ply + 1);

        board.unmakeMove(move);

        // Beta cutoff
        if (score >= beta)
        {
            tt.store(hashKey, beta, TTFlag::LOWER_BOUND, 0);
            return beta;
        }
        // Update alpha
        if (score > alpha)
            alpha = score;
    }
    
    // Detect checkmate within quiescence search
    if (inCheck && moves.size() == 0)
    {
        return -CHECKMATE_SCORE + ply; // Checkmate with distance penalty
    }

    tt.store(hashKey, alpha, TTFlag::UPPER_BOUND, 0);
    return alpha;
}

void ChessEngine::orderMoves(chess::Board &board, chess::Movelist &moves)
{
    // Score each move using heuristics
    for (auto &move : moves)
    {
        scoreMoves(board, move);
    }

    // Sort moves by score (highest first)
    moves.sort();
}

void ChessEngine::scoreMoves(const chess::Board &board, chess::Move &move)
{
    int score = 0;

    // Score captures based on MVV-LVA (Most Valuable Victim, Least Valuable Aggressor)
    if (board.at(move.to()) != chess::Piece::NONE)
    {
        // Get piece types
        chess::PieceType captured = chess::utils::typeOfPiece(board.at(move.to()));
        chess::PieceType attacker = chess::utils::typeOfPiece(board.at(move.from()));

        // MVV-LVA: 6*victim - aggressor + 10 (to ensure captures are considered first)
        score = SEE::getMvvLvaScore(captured, attacker);

        // Add bonus for good captures based on SEE
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
    else if (move.typeOf() == chess::Move::ENPASSANT)
    {
        score = SEE::getMvvLvaScore(chess::PieceType::PAWN,
                                    chess::PieceType::PAWN) +
                1000;
    }

    // Score promotions
    if (move.typeOf() == chess::Move::PROMOTION)
    {
        // Higher score for queen promotions
        if (move.promotionType() == chess::PieceType::QUEEN)
            score += 100000;
        else if (move.promotionType() == chess::PieceType::ROOK)
            score += 500;
        else if (move.promotionType() == chess::PieceType::BISHOP ||
                 move.promotionType() == chess::PieceType::KNIGHT)
            score += 300;
    }

    // We could add more scoring factors here:
    // - Killer moves (quiet moves that caused beta cutoffs at the same depth)
    // - History heuristic (for quiet moves)

    move.setScore(score);
}

int ChessEngine::evaluatePosition(const chess::Board &board)
{
    // The board evaluation is delegated to the Evaluation class
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