#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <cstdint>
#include "types.h"


/* --
-- xorshift64star Pseudo - Random Number Generator
-- For further analysis see http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
-- */
class PRNG {

    uint64_t s;

public:
    PRNG(uint64_t seed) :
        s(seed) {
        assert(seed);
    }

    uint64_t rand64() {
        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

};

/*-- Zobrist hashkeys to hash a position --*/
class Zobrist {

public:
    static Key pieces[COLOR_CNT][SQUARE_CNT][TYPE_CNT];
    static Key black_to_move;
    static Key castling[16];
    static Key enpassant[FILE_CNT];

    void init();

    Zobrist() {
        init();
    }
};

#endif