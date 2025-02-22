#include <utility>
#include <tuple>

#include "search.h"
#include "board.h"
#include "util.h"
#include "engine.h"

#define Pair(a, b)  std::make_pair((a), (b))

// global flag dictating if the search should stop
bool gStopSearching;

struct SearchScope {
    // Total nodes searched
    uint64_t nodes;
    // Total quiscence nodes searched
    uint64_t qnodes;
    // Maximal ply reached by search
    int maxPly;
    // Best move found
    Move bestMove;
    // Total search depth
    int depth;
    // Hash hits during search
    uint64_t hashHitCnt;
    
    // start time of the search
    uint64_t start_time;
    
    // search limits
    Time     timeLimit;
    uint64_t nodesLimit;
    int      depthLimit;
    int srlimits;

} ss;


void updateSearchLimits() {
    
    if ( ss.srlimits & TimeLimit ) {
        auto now = now_time_ms();
        gStopSearching = (now - ss.start_time) >= ss.timeLimit;
    }

    if ( ss.srlimits & NodesLimit ) {
        gStopSearching = ss.nodes >= ss.nodesLimit;
    }
}

// Get the amount of time that should be allocated for the current move for the current side
Time getSearchTime(const Board& b, SearchRequest sr) {
    PieceColor side = b.stm();
    Time moveTime;
    moveTime = std::max(sr.time[side] / 40 + sr.inc[side], 50ULL);
    return moveTime;
}

void start_search(Board& b, ExtMove& result, SearchRequest sr) {
    ss.start_time = now_time_ms();

    ss.srlimits = sr.limits;
    ss.depthLimit = ( sr.limits & DepthLimit ) ? sr.depth : MAX_DEPTH;
    ss.nodesLimit = ( sr.limits & NodesLimit ) ? sr.nodes : 0;
    ss.timeLimit = (sr.movetime && (sr.limits & TimeLimit)) ? sr.movetime : getSearchTime(b, sr);

    // TODO: infinite mode not yet suported
    // if there are no limits or search is set to infinite behave as if we were given 30s per move
    if ( !(sr.limits) || (sr.limits & InfiniteLimit) ) {
        ss.timeLimit = 30000;
        ss.srlimits |= TimeLimit;
        cout << "info string warning time control set to " << ss.timeLimit << endl;
    }

    gStopSearching = false;

    uint64_t nodeCount = search(b, result, 0);
    
    //auto delta_ms = std::max((now_time_ms() - ss.start_time), 1ULL);
    //cout << "info " << "depth " << depth << " seldepth " << ss.maxPly << " score "; print_score(relativeScore);
    //cout << " time " << delta_ms << " nodes " << nodeCount << " nps " << (nodeCount / delta_ms * 1000) << " pv "; print_pv_moves(b); cout << endl;

    cout << "bestmove "; print_move(result.move); cout << endl;
}

SearchResult alphabeta(Board& b, int depth, int ply, Score alpha, Score beta);
SearchResult search_widen(Board& b, int depth, Score val);

uint64_t search(Board & b, ExtMove& result, int depth) {
    Score bestValue;
    Move bestMove;
    
    ss.depth = ss.depthLimit;
    ss.bestMove = Move();
    ss.nodes = 0;
    ss.qnodes = 0;
    ss.maxPly = 0;

    std::tie(bestValue, bestMove) = alphabeta(b, 1, 0, -CHECKMATE_SCORE, CHECKMATE_SCORE);
    for (int depth_iter = 2; depth_iter <= ss.depthLimit; depth_iter++ ) {
        ss.hashHitCnt = 0;
        
        std::tie(bestValue, bestMove) = search_widen(b, depth_iter, bestValue);//alphabeta(b, depth_iter, 0, -CHECKMATE_SCORE, CHECKMATE_SCORE);//search_widen(b, depth_iter, bestValue);
        
        // don't copy the move from the last iteration if the search was aborted
        if ( !gStopSearching ) {
            result.score = bestValue;
            result.move = ss.bestMove;
        } else {
            break;
        }

        auto delta_ms = std::max((now_time_ms() - ss.start_time), 1ULL);

        //std::cout << "info string depth " << depth_iter << " seldepth " << ss.maxPly << " hash-hits " << ss.hashHitCnt << " nodes " << ss.nodes << " qnodes " << ss.qnodes  << " score " << bestValue << " "; print_pv_moves(b) << std::endl;
        if ( gDebug ) {
            cout << "info string " << " hash-hits " << ss.hashHitCnt << " nodes " << ss.nodes << " qnodes " << ss.qnodes << endl;
        }
        cout << "info " << "depth " << depth_iter << " seldepth " << ss.maxPly << " score "; print_score(bestValue);
        cout << " time " << delta_ms << " nodes " << ss.nodes << " nps " << (ss.nodes / delta_ms * 1000) << " pv "; print_pv_moves(b); cout << endl;
    }

    //ss.bestMove = bestMove;
    //result.score = bestValue;
    //result.move = ss.bestMove;

    //return ss.gNodesSearched;
    return ss.nodes;
}

SearchResult search_widen(Board& b, int depth, Score val) {
    Score temp = val,
          alpha = val - 50,
          beta = val + 50;
    
    Move bestMove;  
    std::tie(temp, bestMove) = alphabeta(b, depth, 0, alpha, beta); 

    if ( temp <= alpha || temp >= beta ) {
        std::tie(temp, bestMove) = alphabeta(b, depth, 0, -CHECKMATE_SCORE, CHECKMATE_SCORE);
    }

    return Pair(temp, bestMove);
}


SearchResult alphabeta(Board& b, int depth, int ply, Score alpha, Score beta) {

    Score score = -CHECKMATE_SCORE;     // the best score that the maximizing player can achieve in this node
    Move  bestMove = NullMove;          // the move with the best score
    NodeType nodeType = ALPHA_NODE;     // type of the node we are currently in, used by the transposition table
    int tried_moves = 0;                // count of moves searched in the current node, used by the late move reduction
    int depth_extensions = 0;           // search depth extensions
    bool needs_fuller_search;           // controls search prunning and reductions, dictates whether the fuller search is required

    /*-- let the engine process new inputs and update limits during the search --*/
    if ( ss.nodes & (2048 - 1) ) {
        updateSearchLimits();
    }

    /*-- check if we should end the search now --*/
    if ( gStopSearching ) {
        return Pair(0, NullMove);
    }

    /*-- detect the threefold repetition --*/
    if ( b.detectRepetition() ) {
        return Pair(0, NullMove);
    }

    #ifndef DISABLE_TT
    /*--
    -- Probe the transposition table                                 
    -- if the depth is sufficient and the node type is exact return the score, otherwise return beta
    -- if score > beta for beta nodes and alpha if score < alpha for alpha nodes,
    -- always returns the best move to allow further move ordering
    --*/
    if ( b.ttable()->probe(b.pstate().hash, depth, alpha, beta, score, bestMove) ) {
        ss.hashHitCnt++;
        if ( ply == 0 ) {
            ss.bestMove = bestMove;
        }
        return Pair (score, bestMove);
    }

    #endif // DISABLE_TT

    #ifndef DISABLE_QSEARCH
    /*-- 
    -- Perform a quiescence search at horizont nodes (by tring the capturing moves at leafs)
    -- to establish a quiet position before evaluation
    --*/
    if ( depth == 0 ) {
        ss.nodes++;
        Score quiet = qsearch(b, ply, alpha, beta);
        //!!!(commenting out for now) TODO: slows down search by x3
        // the result and most likely reason is the shortened PV in TT
        //b.ttable()->save(b.pstate().hash, depth, quiet, PV_NODE, bestMove); 
        return Pair( quiet , NullMove );
    }
    
    #else // !DISABLE_QSEARCH
    /*-- no qsearch extensions, straight evaluation at leaf nodes*/
    if ( depth == 0 ) {
        ss.nodes++;
        Score value = evaluate(b);
        //b.ttable()->save(b.pstate().hash, depth, value, PV_NODE, bestMove);
        return Pair(value, NullMove);
    }
    #endif // !DISABLE_QSEARCH
    
    const bool inCheck = IsKingInCheck(b);

    #ifndef DISABLE_NULL_MOVE_PRUNING
     /*-- 
     -- Null Move prunning
     -- Gives the opponent the oportunity to make two moves in a row, it should ruin our possition, i.e score < beta,
     -- but if it can't harm us, in this case we assume making a move wouldn't ruin our possition either.
     -- Doesn't work if the possition is in zugzwang so these positions must be detected and properly constrained.
     --*/
    const bool inEndgame = IsEndgame(b);
    if ( !inCheck && !inEndgame && depth >= 3 ) {
        
        /*-- Depth reduction constant of the null move pruning --*/
        const int R = 2;//(depth > 6) ? 3 : 2; // 2 or 3 for depth > 6

        Move refutationMove;
        b.moveDoNull();
        std::tie(score, refutationMove) = alphabeta(b, depth - 1 - R, ply + 1, -beta, -beta + 1);
        score = -score;
        b.moveUndoNull();
        if ( score >= beta ) {
            //b.ttable()->save(b.pstate().hash, depth, score, BETA_NODE, NullMove); // TODO: not sure if tt save should be done here
            return Pair(score, NullMove);
        }
    }
    #endif // DISABLE_NULL_MOVE_PRUNING

    MoveList moves;
    generate_all_moves(b, moves);

    /*-- Handle checkmates and stalemates*/
    if ( moves.empty() )
        return Pair( evaluateNoMoves(b, ply), NullMove );

    sort_moves(b, depth, moves, bestMove);

    for ( Move mv : moves ) {

        needs_fuller_search = true;
        ss.nodes++;

        //if ( ss.depth == depth ) {
        //    std::cout << " moves:"; print_move(mv) << "\n";
        //}

        Move refutationMove;
        
        b.moveDo(mv);

        #ifndef DISABLE_LMR
        /*-- 
        -- Late Move Reduction 
        -- We think our move ordering is good, so moves further in list are searched with a reduced depth 
        -- and a null window aroung alpha, i.e  [alpha alpha+1], if score > alpha for the mocw we do a fuller search
        --*/
        int DR = 0; // depth of the reduction

        /*-- we have tried 3 moves so far, there is still depth left and we aren't in check --*/
        if ( depth >= 3 && tried_moves > 3 && !inCheck ) {
            /* we are not in a pv node so we can reduce depth */
            if ( nodeType != PV_NODE )
                DR += 1;

            /* we don't give checks and the move is quiet so we can reduce even more */
            if ( !IsKingInCheck(b) && !mv.isCapture() && !mv.isPromotion() )
                DR += 1;
        }

        if ( DR > 0 ) {
            std::tie(score, refutationMove) = alphabeta(b, depth - 1 - DR, ply + 1, -alpha - 1, -alpha);
            score = -score;

            needs_fuller_search = score > alpha;
        }
        #endif // DISABLE_LMR



        #ifndef DISABLE_PVS
        /* --
        -- Primary variation search
        -- We suppose that our first move is the best move, i.e. it raises alpha, and the remaining moves will fail low (i.e score < new alpha),
        -- we can verify this assumption by constraining the window to [alpha-1, alpha] and if score > alpha for this move we do a full search.
        -- */
        if ( nodeType == PV_NODE && needs_fuller_search ) {
            std::tie(score, refutationMove) = alphabeta(b, depth - 1, ply + 1, -alpha - 1, -alpha);
            score = -score;

            needs_fuller_search = score > alpha;
        }
        #endif // DISABLE_PVS

        if ( needs_fuller_search ) {
            std::tie(score, refutationMove) = alphabeta(b, depth - 1, ply + 1, -beta, -alpha);
            score = -score;
        }
        
        b.moveUndo();

        if ( score >= beta ) {
            b.ttable()->save(b.pstate().hash, depth, beta, BETA_NODE, mv);
            return Pair(score, bestMove); // ( the betta cutoff )
        }

        if ( score > alpha ) {
            alpha = score;
            bestMove = mv;
            nodeType = PV_NODE;
            if ( ply == 0 ) {
                ss.bestMove = mv;
            }
        }

        tried_moves++;
    }


    b.ttable()->save(b.pstate().hash, depth, alpha, nodeType, bestMove);
    return Pair(alpha, bestMove);
}

/* --
-- Quiecence search
-- Needed to establish a quiet position (no hanging pieces) befor the board can be evaluated
-- */
Score qsearch(Board& b, int ply, Score alpha, Score beta) {

    Position& pos = b;

    /* get a "stand pat" score */
    Score val = evaluate(pos);
    ss.qnodes++;
    ss.maxPly = std::max(ss.maxPly, ply);

    //return val;

    if ( val >= beta ) return beta;
    if ( val > alpha ) alpha = val;

    // Limit qsearch depth by extra 6 plies
    //if ( (ply - ss.depth) >= 6 )
    //    return val;

    MoveList moveList;
    generate_all_moves(pos, moveList);

    sort_moves(b, ply - ss.depth, moveList, NullMove);

    for ( Move move : moveList ) {

        if ( move.isCapture() ) {

            PieceType piece_origin = b.pieceAt(move.origin()).type();
            PieceType piece_target = b.pieceAt(move.target()).type();

            /* Delta cutoff */
            if ( (val + PieceWeight[piece_target] + 200 < alpha) && !IsEndgame(pos) )
                continue;

            b.moveDo(move);
            Value score = -qsearch(b, ply + 1, -beta, -alpha);
            b.moveUndo();
            
            if ( score >= beta ) return beta;
            if ( score > alpha ) alpha = score;
        }
    }

    return alpha;
}


/* --
-- For cases when there are no legal moves for the current side to make
-- evaluation is either a checkmate or a stalemate
-- */
inline Value evaluateNoMoves(const Position& pos, int ply) {  

    int bestValue;

    // Checkmate
    if ( IsKingInCheck(pos) )
        bestValue = -CHECKMATE_SCORE + ply;
    else    
        bestValue = 0; // Stalemate

    return bestValue;
}


void sort_moves(Board& b, int depth, MoveList& moves, Move bestMove) {
    int size = moves.size();

    for ( int i = 0; i < size; i++ ) {
        SortMove& mv = moves[i];
        
        if ( mv == bestMove ) {
            mv.sort = TT_SORT;
        } else {
            PieceType attacker = b.pieceAt(mv.origin()).type();
            PieceType victim   = b.pieceAt(mv.target()).type();
            mv.sort = MVV_LVA_SORT[victim][attacker];
        }
    }

    std::sort(moves.begin(), moves.end(), [](SortMove a, SortMove b) { return a.sort > b.sort;  } );
}

// 3 fold repetition test position
// 3rk1nr/p4pp1/2Q1p1b1/3pP1Pp/5B1P/NP2PK2/q1P2PB1/7R b k - 8 25