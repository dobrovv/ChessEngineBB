#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

using Bitboard = std::uint64_t;

constexpr Bitboard Empty_bb    = 0;
constexpr Bitboard Universe_bb = ~Empty_bb;

constexpr Bitboard Rank1_bb = 0xFF;
constexpr Bitboard Rank2_bb = SHL(Rank1_bb, 8*1);
constexpr Bitboard Rank3_bb = SHL(Rank1_bb, 8*2);
constexpr Bitboard Rank4_bb = SHL(Rank1_bb, 8*3);
constexpr Bitboard Rank5_bb = SHL(Rank1_bb, 8*4);
constexpr Bitboard Rank6_bb = SHL(Rank1_bb, 8*5);
constexpr Bitboard Rank7_bb = SHL(Rank1_bb, 8*6);
constexpr Bitboard Rank8_bb = SHL(Rank1_bb, 8*7);

constexpr Bitboard FileA_bb = 0x0101010101010101ULL;
constexpr Bitboard FileB_bb = SHL(FileA_bb, 1);
constexpr Bitboard FileC_bb = SHL(FileA_bb, 2);
constexpr Bitboard FileD_bb = SHL(FileA_bb, 3);
constexpr Bitboard FileE_bb = SHL(FileA_bb, 4);
constexpr Bitboard FileF_bb = SHL(FileA_bb, 5);
constexpr Bitboard FileG_bb = SHL(FileA_bb, 6);
constexpr Bitboard FileH_bb = SHL(FileA_bb, 7);

constexpr Bitboard set_bb(Bitboard bboard, Square square){
    return bboard | SHL(1,square);
}

constexpr Bitboard reset_bb(Bitboard bboard, Square square){
    return bboard & ~SHL(1,square);
}

inline Bitboard set_ref_bb(Bitboard& bboard, Square square){
    return bboard |= SHL(1,square);
}

inline Bitboard reset_ref_bb(Bitboard& bboard, Square square){
    return bboard &= ~SHL(1,square);
}

constexpr bool is_set_bb(Bitboard bboard, Square square) {
    return bboard & SHL(1,square);
}

///////////////////////////////////////////
////         Population Count          ////
///////////////////////////////////////////

#ifdef __GNUC__
inline int popcount_bb(Bitboard bboard) {
    return __builtin_popcountll(bboard);
}
#else
//This uses fewer arithmetic operations than any other known
//implementation on machines with fast multiplication.
//It uses 12 arithmetic operations, one of which is a multiply.
const uint64_t m1  = 0x5555555555555555; //binary: 0101...
const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...
inline int popcount_bb(Bitboard x) {
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
    return (x * h01)>>56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
}
#endif

///////////////////////////////////////////
////            Bitscans               ////
///////////////////////////////////////////
#ifdef __GNUC__
inline Square bitscan_forward(Bitboard bboard) {
    assert(bboard != 0);
    return __builtin_ctzll(bboard);
}

inline Square bitscan_reverse(Bitboard bboard) {
    assert(bboard != 0);
    return __builtin_clzll(bboard) ^ 63;
}
#endif


inline Square pop_lsb(Bitboard &bboard) {
    assert(bboard != 0);
    Square square = bitscan_forward(bboard);
    bboard &= bboard - 1;
    return square;
}

/*inline Square pop_msb(Bitboard &bboard) {
    Square square = bitscan_reverse(bboard);
    bboard &= bboard - 1;   // what comes here ?
    return square;
}*/

#endif // BITBOARD_H
