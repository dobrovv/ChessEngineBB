#include "zobrist.h"

void Zobrist::init() {

    PRNG rng(6960111479);

    // Zobrist keys of each color for each piece type on each square
    for (int c = 0; c < COLOR_CNT; c++)
        for (int sq = 0; sq < SQUARE_CNT; sq++)
            for (int t = Pawn; t < TYPE_CNT; t++)
                pieces[c][sq][t] = rng.rand64();

    // All permutations of castling rights
    for (int cs = 0; cs < 16; cs++) {
        castling[cs] = rng.rand64();
    }

    // File of the enpassant square
    for (int f = 0; f < FILE_CNT; f++)
        enpassant[f] = rng.rand64();

    black_to_move = rng.rand64();
}

Zobrist gInitializerZobrist; // Singleton, to initialize zobrist key bitstrings
