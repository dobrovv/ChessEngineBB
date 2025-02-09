#include "search.h"
#include "board.h"

#include <utility>
#include <tuple>

// Total nodes searched
uint64_t gNodesSearched;
// Total quiscence nodes searched
uint64_t gQNodesSearched;
// Maximal ply reached by search
int gMaxPly;
// Best move found
Move gBestMove;
// Total search depth
int gDepth;
// Hash hits during search
uint64_t gHashHitCnt;

std::pair<Value, Move> alphabeta(Board& b, int depth, int ply, Value alpha, Value beta);


uint64_t search(Board & b, ExtMove& result, int depth) {
    Value bestValue;
    Move bestMove;

    gDepth = depth;
    gMaxPly = 0;
    gHashHitCnt = 0;
    gBestMove = Move();
    gNodesSearched = 0;
    gQNodesSearched = 0;

    std::tie(bestValue, bestMove) = alphabeta(b, depth, 0, -INT_MAX, INT_MAX);
    
    bestMove = gBestMove;

    result.score = bestValue;
    result.move = bestMove;

    std::cout << "info string hash-hits " << gHashHitCnt << " qnodes " << gQNodesSearched << " max-plies " << gMaxPly << std::endl;

    //return gNodesSearched;
    return gQNodesSearched;
}


std::pair<Value, Move> alphabeta(Board& b, int depth, int ply, Value alpha, Value beta) {

    /* if (depth == 0) {
         gNodesSearched++;
         return b.evaluate();
     }*/

    bool maximazingPlayer = b.sideToMove() == White;

    // Perform quiescence search
    if ( depth == 0 ) {
        gNodesSearched++;
        return std::make_pair( qsearch(b, ply, alpha, beta), Move() );
    }

    MoveList moves;
    generate_all_moves(b, moves);

    if ( moves.empty() )
        return std::make_pair( evaluateNoMoves(b, depth), Move() );

    Value value;
    Move bestMove;
    Move moveBellow;


    if ( maximazingPlayer ) {
        value = -INT_MAX; // note INT_MIN = -INT_MAX-1
        for ( Move mv : moves ) {

            b.moveDo(mv);

            Key curMoveHash = b.positionState().hash;
            TTEntry* hashed = b.table->cell(curMoveHash);
            Value alphaBeta;

            if ( hashed->key == curMoveHash && hashed->depth >= depth ) {
                alphaBeta = hashed->value;
                //bestMove = hashed->move;
                gHashHitCnt++;
            } else {
                std::tie(alphaBeta, moveBellow) = alphabeta(b, depth - 1, ply + 1, alpha, beta);         // value = std::max(value, alphabeta(b, depth - 1 , alpha, beta, false));
                b.table->save(curMoveHash, alphaBeta, depth, moveBellow);
            }

            if ( value < alphaBeta ) {
                value = alphaBeta;
                if ( gDepth == depth ) {
                    gBestMove = mv;
                }
                bestMove = mv;
            }
            alpha = std::max(alpha, value);
            b.moveUndo();

            if ( value >= beta )
                return std::make_pair(value, bestMove); // ( betta cutoff )
        }
    } else {
        value = +INT_MAX;
        for ( Move mv : moves ) {

            b.moveDo(mv);

            Key curMoveHash = b.positionState().hash;
            TTEntry* hashed = b.table->cell(curMoveHash);
            Value alphaBeta;

            if ( hashed->key == curMoveHash && hashed->depth >= depth ) {
                alphaBeta = hashed->value;
                //bestMove = hashed->move;
                gHashHitCnt++;
            } else {
                std::tie(alphaBeta, moveBellow) = alphabeta(b, depth - 1, ply + 1, alpha, beta);
                b.table->save(curMoveHash, alphaBeta, depth, moveBellow);
            }

            if ( value > alphaBeta ) {                                      //value = std::min(value, alphabeta(b, depth - 1, alpha, beta, true));
                value = alphaBeta;
                if ( gDepth == depth )
                    gBestMove = mv;
                bestMove = mv;
            }
            beta = std::min(beta, value);
            b.moveUndo();

            if ( value <= alpha )
                return std::make_pair(value, bestMove); // ( alpha cutoff )
        }
    }

    return std::make_pair(value, bestMove);
}


// Quiescence search
Value qsearch(Board& b, int ply, Value alpha, Value beta) {

    Position& pos = b;
    bool maximazingPlayer = b.sideToMove() == White;

    /* get a "stand pat" score */
    Value val = evaluate(pos);
    gQNodesSearched++;
    gMaxPly = std::max(gMaxPly, ply);

    //return val;

    if ( maximazingPlayer ) {     // maximizing player
        if ( val >= beta ) return beta;
        if ( alpha < val ) alpha = val;
    } else {                    // minimizing player
        if ( val <= alpha ) return alpha;
        if ( val < beta )   beta = val;
    }

    // Limit qsearch depth by extra 12 plies
    if ( (ply - gDepth) >= 6 )
        return val;

    MoveList moveList;
    generate_all_moves(pos, moveList);

    for ( Move move : moveList ) {

        if ( move.isCapture() ) {

            PieceType piece_origin = b.pieceAt(move.origin()).type();
            PieceType piece_target = b.pieceAt(move.target()).type();

            /* Delta cutoff */
            if ( maximazingPlayer ) {
                if ( (val + PieceWeight[piece_target] + 200 < alpha) && !IsEndgame(pos) )
                    continue;
            } else {
                if ( (val - PieceWeight[piece_target] - 200 > beta) && !IsEndgame(pos) )
                    continue;
            }

            if ( piece_origin == Pawn ) {
                // Pawn captures never loose material

            } else if ( PieceWeight[piece_origin] - PieceWeight[piece_target] - 20 > 0 && isDefended(pos, !pos.sideToMove(), move.target()) ) {
                // More valuable piece attacks a less valuable defended piece
                continue;
            }


            b.moveDo(move);
            Value score = qsearch(b, ply + 1, alpha, beta);
            b.moveUndo();

            if ( maximazingPlayer ) {   // maximizing player
                if ( score >= beta ) return beta;
                if ( score > alpha ) alpha = score;
            } else {                    // minimizing player
                if ( score <= alpha ) return alpha;
                if ( score < beta ) beta = score;
            }
        }
    }

    return maximazingPlayer ? alpha : beta;
}


// For cases with no more legal moves for the current side to make
// evaluates to either a checkmate or a stalemate
inline Value evaluateNoMoves(const Position& pos, int depth) {

    int bestValue;

    if ( pos.sideToMove() == White ) {
        if ( isKingAttacked<White>(pos) ) {   // Checkmate
            bestValue = -INT_MAX + (gDepth - depth);
        } else {                            // Stalemate
            bestValue = 0;
        }
    } else {
        if ( isKingAttacked<Black>(pos) ) {   // Checkmate
            bestValue = INT_MAX + (-gDepth + depth);
        } else {                              // Stalemate
            bestValue = 0;
        }
    }

    return bestValue;
}


// Endgame detection is based solely on the amount of material left 
bool IsEndgame(const Position& pos) {
    int32_t materialValue = 0;

    for ( PieceType pt = Pawn; pt < King; pt = PieceType(pt + 1) ) {
        materialValue += PieceWeight[pt] * (popcount_bb(pos.pieces(!pos.sideToMove(), pt)));
    }
    return materialValue <= 1300;
}