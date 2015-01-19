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

template <Direction dir> inline
Bitboard shift_bb(const Bitboard bboard) {

    if (dir == NorthWest) {
        return SHL(bboard & ~FileA_bb, 7);
    } else if (dir == North) {
        return SHL(bboard, 8);
    } else if (dir == NorthEast) {
        return SHL(bboard & ~FileH_bb, 9);
    } else if (dir == East) {
        return SHL(bboard & ~FileH_bb, 1);
    } else if (dir == SouthEast) {
        return SHR(bboard & ~FileH_bb, 7);
    } else if (dir == South) {
        return SHR(bboard, 8);
    } else if (dir == SouthWest) {
        return SHR(bboard & ~FileA_bb, 9);
    } else if (dir == West) {
        return SHR(bboard & ~FileA_bb, 1);
    }
}

///////////////////////////////////////////
////         Population Count          ////
///////////////////////////////////////////

#ifdef __GNUC__
inline int popcount_bb(Bitboard bboard) {
    return __builtin_popcountll(bboard);
}
#endif


///////////////////////////////////////////
////            Bitscans               ////
///////////////////////////////////////////
#ifdef __GNUC__
inline Square lsb_bb(Bitboard bboard) {
    assert(bboard != 0);
    return __builtin_ctzll(bboard);
}

inline Square msb_bb(Bitboard bboard) {
    assert(bboard != 0);
    return __builtin_clzll(bboard) ^ 63;
}
#endif


inline Square pop_lsb_bb(Bitboard &bboard) {
    assert(bboard != 0);
    Square square = lsb_bb(bboard);
    bboard &= bboard - 1;
    return square;
}

/*inline Square pop_msb_bb(Bitboard &bboard) {
    Square square = bitscan_reverse(bboard);
    bboard &= bboard - 1;   // what comes here ?
    return square;
}*/

#define foreach_pop_lsb(square, bboard) for (Square square; (bboard) && ((square = pop_lsb_bb(bboard)) || true);)

#endif // BITBOARD_H
