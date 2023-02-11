#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"
#include <bitset>   // popcount
#include <climits>  // popcount

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

#elif _MSC_VER
// or _mm_popcnt_u64 with SSE4 and <nmmintrin.h>
#include <intrin.h>
inline int popcount_bb(Bitboard bboard) {
    return __popcnt64(bboard);
}

#else

inline int popcount_bb(Bitboard n) {
    return std::bitset<CHAR_BIT * sizeof n>(n).count();
}

#endif


///////////////////////////////////////////
// Bitscans Forward(lsb) and Reverse(msb)//
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

#elif _MSC_VER
#include <intrin.h>
inline Square lsb_bb(Bitboard bboard) {
    assert(bb != 0);
    return _tzcnt_u64(bboard);
}

inline Square msb_bb(Bitboard bboard) {
    assert(bb != 0);
    return _lzcnt_u64(bboard) ^ 63;

}
#else
/* Generic implementation of bitscan forward and bitscan reverse
 * Ref: https://www.chessprogramming.org/BitScan
 */
const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
inline Square lsb_bb(Bitboard bb) {
    assert(bb != 0);
    constexpr Bitboard debruijn64 = 0x03f79d71b4cb0a89UL;
    return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}

/**
 * bitScanReverse
 * @authors Kim Walisch, Mark Dickinson
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of most significant one bit
 */
inline Square msb_bb(Bitboard bb) {
    assert(bb != 0);
    constexpr Bitboard debruijn64 = 0x03f79d71b4cb0a89UL;
    bb |= bb >> 1;
    bb |= bb >> 2;
    bb |= bb >> 4;
    bb |= bb >> 8;
    bb |= bb >> 16;
    bb |= bb >> 32;
    return index64[(bb * debruijn64) >> 58];
}
#endif


inline Square pop_lsb_bb(Bitboard &bboard) {
    Square square = lsb_bb(bboard);
    bboard &= bboard - 1;
    return square;
}

inline Square pop_msb_bb(Bitboard &bboard) {
    Square square = msb_bb(bboard);
    bboard &= ~(1<<square);
    return square;
}

//TODO: (?) replace square by uint8_t
#define foreach_pop_lsb(square, bboard) for (Square square; (bboard) && ((square = pop_lsb_bb(bboard)) || true);)

#endif // BITBOARD_H
