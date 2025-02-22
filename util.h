#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <sstream>
#include <chrono>

#include "types.h"
#include "board.h"

/*-- current time since epoch in ms --*/
#define now_time_ms() (uint64_t)(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())

using std::ostream;
using std::cout;
using std::endl;

uint64_t perft(Board& b, int depth);

uint64_t divide(Board& b, int depth);

ostream& print_board(const Board& b, ostream& os = std::cout);

// Prints the board with labeled row ranks and column files
ostream& print_board_with_files(const Board& b, ostream& os = std::cout);

ostream& print_bb(const Bitboard& bitboard, ostream& os = std::cout);

ostream& print_square(const Square& square, ostream& os = std::cout);

ostream& print_move(const Move& move, ostream& os = std::cout);

// prints the score as cp <score> or mate <mated-in>
ostream& print_score(Score score, ostream& os = std::cout);

/* Verifies that the given move in UCI notation is legal and can be played,  returns a legal engine move */
Move verify_move(Board& b, std::string moveStr);

// Gets a random legal move
Move randomMove(Board& b);

// print the principle variation line
ostream& print_pv_moves(Board& b, ostream& os = std::cout);

// Get the primary variation line from TT
std::vector<Move> getPrimaryVariation(Board& b);

inline bool IsCheckMateScore(Score val) {
	return std::abs(val) > CHECKMATE_SCORE - 100;
}

/*-- returns mate in x value, mated value is returned in moves not in plies, hence / 2 --*/
inline int value_mated_in(Score val) {
	if ( val > 0 )
		return ( CHECKMATE_SCORE - val ) / 2;
	else
		return ( - val - CHECKMATE_SCORE ) / 2;
}

#endif