#include "position.h"
#include "movegen.h"
#include <cstring> // memset()

/*-- Precomputed Bitboards used as shortcuts during move generation --*/
Bitboard pawnCaptureStepsBB[COLOR_CNT][SQUARE_CNT];
Bitboard kingStepsBB[SQUARE_CNT];
Bitboard knightStepsBB[SQUARE_CNT];
Bitboard directionStepsBB[SQUARE_CNT][DIRECTION_CNT];
Direction fromToDirection[SQUARE_CNT][SQUARE_CNT];

/*-- Precomputed Zobrist keys used to compute hashcode of the current position --*/
Key Zobrist::pieces[COLOR_CNT][SQUARE_CNT][TYPE_CNT];
Key Zobrist::black_to_move;
Key Zobrist::castling[16];
Key Zobrist::enpassant[FILE_CNT];


Position::Position()
{
    side = White;
    occupied_bb = Empty_bb;
    std::memset(colored_bb, 0, sizeof(colored_bb) );
    std::memset(pieces_bb,  0, sizeof(pieces_bb) );
    std::memset(piece_at,    0, sizeof(piece_at) );

    state.castle_rights = NoCastlingFlags;
    state.epSquare = Square(a1);    // a1 == no En passant
    state.halfmove_clock = 0;
}

void Position::movesForSide(PieceColor color, MoveList &moveList)
{
    Position& pos = *this;
    generate_all_moves(pos, moveList);
}