#ifndef ENGINE_H
#define ENGINE_H

#include "types.h"
#include "movegen.h"

/* --
-- Types of the search limits declared for the following search
--*/
enum SearchRequestLimit : uint8_t {
	NoLimit = 0,
	TimeLimit = SHL(1, 0),
	DepthLimit = SHL(1, 1),
	NodesLimit = SHL(1, 2),
	InfiniteLimit = SHL(1, 3),
	MateLimit = SHL(1, 4)
};

/* --
-- Type of the search requests 
-- to the engine sent by the UCI interface
--*/
struct SearchRequest {
	Time		time[COLOR_CNT] = { 0,0 };
	Time		inc[COLOR_CNT] = { 0,0 };
	Time		movetime = 0;
	int			movestogo = 0;
	int			depth = 0;
	uint64_t	nodes = 0;
	int			mate= 0;
	bool		infinite = false;
	bool		ponder = false;
	MoveList	searchmoves = MoveList();
	int limits = 0;

	SearchRequest() {
		time[White] = 0;
		time[Black] = 0;
		inc[White] = 0;
		inc[Black] = 0;
		depth = 0;
		nodes = 0;
		mate = 0;
		infinite = false;
		ponder = false;
		searchmoves = MoveList();
		limits = 0;
	}
};



#endif