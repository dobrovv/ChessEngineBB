#ifndef TT_H
#define TT_H

#include "types.h"

struct TTEntry {
	Key key;
	Value value;
	int depth;
	Move move; //refutation move

};

/*---------------------
-- Transpotion table --
---------------------*/

class TT {
	TTEntry* table;
	
	uint64_t size; 

public:
	TT(int mbSize) {
		size = mbSize * 1024 * 1024;
		table = static_cast<TTEntry*>(malloc(size));
		memset(table, 0, size);

		size /= sizeof(TTEntry);
	}

	~TT() {
		free(table);
	}

	void save(Key key, int value, int depth, Move bestMove) {
		TTEntry* entry = &table[key % size];
		entry->key = key;
		entry->value = value;
		entry->depth = depth;
		entry->move = bestMove;
	}

	bool contains(Key key) {
		TTEntry* entry = &table[key % size];
		return entry->key == key;
	}

	TTEntry* cell(Key key) {
		TTEntry* entry = &table[key % size];
		return entry;
	}

};

#endif