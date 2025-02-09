#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include "types.h"

class PRNG {

    uint64_t s;

public:
    PRNG(uint64_t seed) :
        s(seed) {
        assert(seed);
    }

    // xorshift64star Pseudo-Random Number Generator
    // This class is based on original code written and dedicated
    // to the public domain by Sebastiano Vigna (2014).
    // It has the following characteristics:
    //
    //  -  Outputs 64-bit numbers
    //  -  Passes Dieharder and SmallCrush test batteries
    //  -  Does not require warm-up, no zeroland to escape
    //  -  Internal state is a single 64-bit integer
    //  -  Period is 2^64 - 1
    //  -  Speed: 1.60 ns/call (Core i7 @3.40GHz)
    //
    // For further analysis see
    //   <http://vigna.di.unimi.it/ftp/papers/xorshift.pdf>

    uint64_t rand64() {
        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

};

// Generates Zobrist keys for hashing a position.

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