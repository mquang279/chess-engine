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

    bool inEndgame = isEndGame(board);
    bool inCheck   = board.inCheck();
    bool possibleZ = isPossibleZugzwang(board);

    int eval = evaluation.evaluate(board);
    int materialAdv = std::abs(eval) / 100;


    //Null move pruning
    if (depth >= 3 && !inCheck && hasNonPawnMaterial(board) && !possibleZ && std::abs(eval) < 9000) 
    {
        // Static Null Move Pruning (SNMP) - early pruning based on static evaluation margin
        if (eval >= beta + 120 * depth) {
            return beta;  // Static null move pruning cutoff
        }
        
        // Adaptive reduction based on position evaluation and material advantage
        int R = 3 + depth / 4 + std::min(3, materialAdv / 200); 
        if (inEndgame) R = std::max(2, R - 1);
        if (materialAdv > 500) R++; // Extra reduction with big material advantage
        R = std::min(R, depth - 1); // Don't reduce beyond depth 1
        R = std::min(R, 4);         // Maximum reduction of 4 plies
        
        board.makeNullMove();
        int nullScore = -negamax(board, depth - 1 - R, -beta, -beta + 1, ply + 1, nodes); 
        board.unmakeNullMove();

        if (nullScore >= beta) {
            // Enhanced verification search for positions that might be zugzwang
            if (depth >= 5 && (inEndgame || std::abs(eval - beta) < 100)) {
                // Use a deeper verification search for critical positions
                if (verifyNullMovePrune(board, depth, beta, nodes)) {
                    return beta;
                }
            } 
            else {
                // More confident positions - can safely return beta
                return beta;
            }
        }
    }

    // Order moves for better pruning
    orderMoves(board, moves);

    int bestScore = -INF;
    int alphaOriginal = alpha;
    chess::Move bestMove = chess::Move::NULL_MOVE;

    // Static evaluation for pruning decisions
    int staticEval = evaluatePosition(board);
    bool improving = false;

    // Track if position is improving compared to previous positions
    // This helps make pruning more accurate
    if (depth >= 2)
    {
        improving = staticEval > alpha;
    }

    // Calculate maximum number of moves to consider before pruning
    // More moves allowed at deeper depths
    int lmpLimit = LMP_BASE + LMP_DEPTH_FACTOR * depth;

    // Calculate futility margin based on depth and improving status
    int futilityMargin = FUTILITY_MARGIN_BASE + FUTILITY_MARGIN_MULTIPLIER * depth;
    if (!improving)
    {
        futilityMargin += FUTILITY_MARGIN_BASE; // Higher margin for non-improving positions
    }

    // Iterate through each move
    for (int i = 0; i < moves.size(); i++)
    {
        chess::Move move = moves[i];

        bool isCapture = board.at(move.to()) != chess::Piece::NONE;
        bool isPromotion = move.typeOf() == chess::Move::PROMOTION;

        // Late Move Pruning (LMP) - Skip quiet moves after trying several
        // Only for shallow depths and quiet moves
        if (depth <= 3 && i >= lmpLimit && !isCapture && !isPromotion && !board.inCheck())
        {
            continue; // Skip this quiet move entirely
        }

        // Make the move
        board.makeMove(move);

        // Check if this move gives check
        bool givesCheck = board.inCheck();


        // Futility Pruning - Skip moves unlikely to improve alpha
        if (depth <= 3 && !isCapture && !isPromotion && !givesCheck && !board.inCheck())
        {
            // Skip moves that can't possibly improve alpha even with a generous margin
            if (staticEval + futilityMargin <= alpha)
            {
                board.unmakeMove(move);
                continue;
            }
        }

        // Late Move Reduction (LMR) - Search quiet later moves with reduced depth
        bool isReduced = false;
        int newDepth = depth - 1;

        // Apply LMR for quiet moves after we've searched several promising moves
        if (depth >= LMR_MIN_DEPTH && i >= LMR_MIN_MOVES && !isCapture && !isPromotion && !givesCheck && !board.inCheck())
        {
            // Calculate reduction amount based on depth and move index
            // Later moves get reduced more, deeper searches get reduced more
            int reduction = 1 + (depth / 6) + (i / 6);

            // Cap reduction to avoid over-reduction
            if (reduction >= depth)
                reduction = depth - 1;

            // Less reduction in improving positions
            if (improving && reduction > 1)
                reduction--;

            isReduced = true;
            newDepth = depth - reduction;

            // Ensure minimum search depth of 1
            if (newDepth < 1)
                newDepth = 1;
        }
        // Recursive negamax call with negated bounds
        int score;

        // If reduced search, do a null-window search first
        if (isReduced)
        {
            score = -negamax(board, newDepth, ply + 1, -alpha - 1, -alpha, nodes);

            // If the reduced search returns a promising score, search again with full depth
            if (score > alpha)
            {
                score = -negamax(board, depth - 1, ply + 1, -beta, -alpha, nodes);
            }
        }
        else
        {
            // Full-window search for promising moves
            score = -negamax(board, newDepth, ply + 1, -beta, -alpha, nodes);
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

bool ChessEngine::hasNonPawnMaterial(const chess::Board &board) const
{
    chess::Color side = board.sideToMove();

    // Non-pawn material: knight, bishop, rook, queen
    chess::Bitboard pieces = board.pieces(chess::PieceType::KNIGHT, side) |
                             board.pieces(chess::PieceType::BISHOP, side) |
                             board.pieces(chess::PieceType::ROOK, side) |
                             board.pieces(chess::PieceType::QUEEN, side);

    return pieces != 0;
}


bool ChessEngine::isEndGame(const chess::Board &board) const
{
    chess::Color us = board.sideToMove();
    chess::Color them = (us == chess::Color::WHITE ? chess::Color::BLACK : chess::Color::WHITE);

    int ourCount = chess::builtin::popcount(
        board.pieces(chess::PieceType::KNIGHT, us) |
        board.pieces(chess::PieceType::BISHOP, us) |
        board.pieces(chess::PieceType::ROOK, us) |
        board.pieces(chess::PieceType::QUEEN, us));

    int theirCount = chess::builtin::popcount(
        board.pieces(chess::PieceType::KNIGHT, them) |
        board.pieces(chess::PieceType::BISHOP, them) |
        board.pieces(chess::PieceType::ROOK, them) |
        board.pieces(chess::PieceType::QUEEN, them));

    bool noQueens = (board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE) == 0 &&
                     board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK) == 0);

    return noQueens || (ourCount < 3 && theirCount < 3);
}


bool ChessEngine::isPossibleZugzwang(const chess::Board &board) const
{
    chess::Color us = board.sideToMove();

    // Count non-pawn pieces and pawns
    chess::Bitboard nonPawn = board.pieces(chess::PieceType::KNIGHT, us) |
                              board.pieces(chess::PieceType::BISHOP, us) |
                              board.pieces(chess::PieceType::ROOK, us) |
                              board.pieces(chess::PieceType::QUEEN, us);
    int pieceCount = chess::builtin::popcount(nonPawn);
    int pawnCount  = chess::builtin::popcount(board.pieces(chess::PieceType::PAWN, us));

    // Zugzwang when only king+pawns and very few total material
    bool noMajor = (board.pieces(chess::PieceType::ROOK, us) == 0 &&
                    board.pieces(chess::PieceType::QUEEN, us) == 0);
    if (noMajor && (pieceCount + pawnCount) <= 5) {
        return true;
    }

    return false;
}

bool ChessEngine::verifyNullMovePrune(chess::Board &board, int depth, int beta, uint64_t &nodes)
{
    // Use a shallower search to verify that null move pruning is safe
    int verificationDepth = std::max(3, depth / 2);
    
    // For safer verification, we use a slightly narrower window
    int verificationBeta = beta;
    int verificationAlpha = beta - 1;
    
    // Generate only forcing moves for verification
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);
    
    // Sort the moves to improve pruning
    orderMoves(board, moves);
    
    // Try only a few most promising moves in verification search
    // This is more efficient than a full search
    const int MAX_VERIFY_MOVES = 5;
    int movesToTry = std::min(MAX_VERIFY_MOVES, (int)moves.size());
    
    for (int i = 0; i < movesToTry; i++)
    {
        board.makeMove(moves[i]);
        int score = -negamax(board, verificationDepth - 1, 1, -verificationBeta, -verificationAlpha, nodes);
        board.unmakeMove(moves[i]);
        
        // If any move beats beta, verification succeeds
        if (score >= verificationBeta)
        {
            return true;
        }
    }
    
    // Verification failed
    return false;
}
