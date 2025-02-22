#ifndef TT_H
#define TT_H

#include "types.h"

using u8 = std::uint8_t;

/*---------------------
-- Transpotion Table
-- Details at https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm
---------------------*/

/*--
 -- Types of TT nodes
 --*/

enum NodeType : std::uint8_t {
	PV_NODE,		// PV Node
	ALPHA_NODE,		// All Node
	BETA_NODE,		// Cut-Node
};

/*--
 -- Entry in TT
 --*/

struct TTEntry {
	Key hashkey;
	int depth;
	Score score;
	NodeType type;
	Move ttMove; //refutation move
};



/*--

--*/
class TT {
	TTEntry* table;
	
	std::uint64_t size; 

public:
	TT(std::uint64_t sizeMb) {
		size = sizeMb * 1024 * 1024;
		table = static_cast<TTEntry*>(malloc(size));
		memset(table, 0, size);

		size /= sizeof(TTEntry);
	}

	~TT() {
		free(table);
	}

	/*-- Save information about a node --*/
	inline void save(Key key, int depth, Score score, NodeType type, Move bestMove) {
		#ifndef DISABLE_TT
		TTEntry* entry = &table[key % size];

		if ( entry->hashkey == key && entry->depth > depth ) return;

		entry->hashkey = key;
		entry->depth = depth;
		entry->score = score;
		entry->type = type;
		entry->ttMove = bestMove;
		#endif
	}

	/* -- 
	-- Probes the transposition table and returns the best move as arg
	-- Checks if the value matches its window and the node type and if it does returns the score
	--*/
	inline bool probe(Key key, int depth, Score alpha, Score beta, Score& val, Move& bestMove) const {
		TTEntry* entry = &table[key % size];
		if ( entry->hashkey == key ) {
			bestMove = entry->ttMove;
			
			if ( entry->depth >= depth ) {
				if ( entry->type == PV_NODE ) {
					val = entry->score;
					return true;
				} else if ( entry->type == ALPHA_NODE && entry->score <= alpha ) {
					val = alpha;
					return true;
				} else if ( entry->type == BETA_NODE && entry->score >= beta ) {
					val = beta;
					return true;
				}
			}
		} else {
			bestMove = NullMove;
		}

		return false;
	}

	/*-- Retrieve a hash entry */
	TTEntry* entry(Key key) {
		TTEntry* entry = &table[key % size];
		return entry;
	}

};

#endif