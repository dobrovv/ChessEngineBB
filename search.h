#include <iostream>

#include "board.h"
#include "movegen.h"

// Determines if the endgame stage is reached
bool IsEndgame(const Position& pos);

// Search entry point
uint64_t search(Board& b, ExtMove& result, int depth);

// Quiecence search
// Needed to establishe a quite position on the board before evaluation 
Value qsearch(Board& b, int ply, Value alpha, Value beta);

// Main search algo
std::pair<Value, Move> alphabeta(Board& b, int depth, int ply, int alpha, int beta);

// For cases with no more legal moves for the current side to make
// evaluates to either a checkmate or a stalemate
inline Value evaluateNoMoves(const Position& pos, int depth);