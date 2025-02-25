#include <iostream>

#include "board.h"
#include "movegen.h"
#include "engine.h"

/*-- Disables heuristics and extensions used by the search--*/

//#define DISABLE_QSEARCH
//#define DISABLE_NULL_MOVE_PRUNING
//#define DISABLE_PVS
//#define DISABLE_LMR

/*-- Maximal depth reached by the engine --*/
#define MAX_DEPTH 255

/*------------
 -- MVA LVV --
 ------------*/

 /*--
 -- Move ordering based on 
 -- the Most valuable victim - Least valuable attacker (MVV-LVA)
 -- a static table for move ordering read as MVV_LVA_SORT[victim][attacker]
 -- details for MVV-LVA https://rustic-chess.org/search/ordering/mvv_lva.html
 --*/
const Sort MVV_LVA_SORT[TYPE_CNT][TYPE_CNT] = {
//    E   P   N   B   R   Q   K		<- Attacker
	{ 0,  0,  0,  0,  0,  0,  0 },	// victim Empty
	{ 0, 15, 14, 13, 12, 11, 10 },	// victim Pawn
	{ 0, 25, 24, 23, 22, 21, 20 },	// victim Knight
	{ 0, 35, 34, 33, 32, 31, 30 },	// victim Bishop
	{ 0, 45, 44, 43, 42, 41, 40 },  // victim Rook
	{ 0, 55, 54, 53, 52, 51, 50 },	// victim Queen
	{ 0,  0,  0,  0,  0,  0,  0 }	// victim King
};

/*-- Score of the TT saved move --*/
const Sort TT_SORT = 255;

using SearchResult = std::pair<Value, Move>;

/*-- Material score defining the beginning of the endgame phase of the game --*/
#define ENDGAME_MAX_MATERIAL 1300;

/* -- 
-- Detect if the endgame phase of the game is reached
-- Endgame detection is based on the amount of material left on the opposite side 
-- */
bool inline IsEndgame(const Position& pos) {
	int32_t materialValue = 0;

	for ( PieceType pt = Pawn; pt < King; pt = PieceType(pt + 1) ) {
		materialValue += PieceWeight[pt] * (popcount_bb(pos.pieces(!pos.stm(), pt)));
	}
	return materialValue <= ENDGAME_MAX_MATERIAL;
}

// Search entry point
void start_search(Board& b, ExtMove& result, SearchRequest sm);

// Root search 
uint64_t search(Board& b, ExtMove& result, int depth);

/* --
-- Quiecence search
-- Needed to establish a quiet position (no hanging pieces) befor the board can be evaluated
-- */
Score qsearch(Board& b, int ply, Value alpha, Value beta);

// Main search algorithm
SearchResult alphabeta(Board& b, int depth, int ply, int alpha, int beta);

/* -- 
-- For cases when there are no legal moves for the current side to make
-- evaluation is either a checkmate or a stalemate
-- */
inline Value evaluateNoMoves(const Position& pos, int ply);

// sort moves
void sort_moves(Board& b, int depth, MoveList& moves, Move bestMove);