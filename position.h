#ifndef POSITION_H
#define POSITION_H

#include "types.h"
#include "bitboard.h"
#include "zobrist.h"

#include <iostream>

enum CastlingRights : std::uint8_t {
    NoCastlingFlags = 0,
    CastlingFlagWQ = SHL(1,0),
    CastlingFlagWK = SHL(1,1),
    CastlingFlagBQ = SHL(1,2),
    CastlingFlagBK = SHL(1,3)
};

const Square NOT_ENPASSANT(a1); // invalid En Passant square
const Square NOT_ENPASSANT_SQ(a1); // invalid En Passant square

struct PositionState {
    std::uint8_t castle_rights;
    Square epSquare;                // En Passant square, a1 == invalid
    std::uint8_t halfmove_clock;    // half move clock;
    Key hash;                       // zobrist hash
};

class MoveList;

class Position
{
protected:
    Bitboard occupied_bb;
    Bitboard colored_bb[COLOR_CNT];
    Bitboard pieces_bb[COLOR_CNT][TYPE_CNT];

    Piece  piece_at[SQUARE_CNT];

    PositionState state;
    PieceColor side;        // sideToMove i.e. next player to move


    // Optimisations
    Bitboard pinned_bb;    // Note, must be recomputed at each move for the current side to move

public:
    Position();

    inline Piece pieceAt(Square square)   const { return piece_at[square]; }
    inline void setPiece(Piece piece, Square square);
    inline void setPiece(PieceColor color, PieceType type, Square square);
    inline void removePiece(Square square);    

    inline Bitboard occupied() const { return occupied_bb; }
    inline Bitboard colored(PieceColor withColor)            const { return colored_bb[withColor]; }
    inline Bitboard pieces(PieceColor color, PieceType type) const { return pieces_bb[color][type]; }

    inline Square enPassantSq() const { return state.epSquare; }
    inline std::uint8_t castling() const { return state.castle_rights; }
    std::uint8_t halfmove_clock() const { return state.halfmove_clock; }
    Key hash() const { return state.hash; }
    
    inline Bitboard pinned() const { return pinned_bb; }
    inline void setPinned(Bitboard newPinned) { pinned_bb = newPinned; }

    inline PieceColor sideToMove() const { return side;}
    inline PieceColor stm() const { return side; }
    inline bool blacks_turn() const { return side; }

    inline void setSideToMove(PieceColor color) { side = color; }    

    inline PositionState positionState() const { return state; }
    inline PositionState pstate() const  { return this->state; }

    inline void setPositionState(PositionState positionState) { state = positionState;}

    //bool isOccupied(Square square) const { return is_set_bb(occupied_bb, square); }

    void movesForSide(PieceColor color, MoveList& moveList);
    //void movesForPawns(PieceColor color, std::vector<Move> &moveList);

private:    /* helper functions */

protected:

};

inline void Position::setPiece(Piece piece, Square square)
{
    assert(piece.type() != Empty);

    const PieceColor color = piece.color();

    set_ref_bb(occupied_bb, square);
    set_ref_bb(colored_bb[color], square);
    set_ref_bb(pieces_bb[color][piece.type()], square);

    piece_at[square] = piece;

    state.hash ^= Zobrist::pieces[color][square][piece.type()];
}

inline void Position::setPiece(PieceColor color, PieceType type, Square square)
{
    assert(type != Empty);

    set_ref_bb(occupied_bb, square);
    set_ref_bb(colored_bb[color], square);
    set_ref_bb(pieces_bb[color][type], square);

    piece_at[square] = Piece(color, type);

    state.hash ^= Zobrist::pieces[color][square][type];
}

inline void Position::removePiece(Square square)
{
    assert(piece_at[square].type() != Empty);

    const Piece p = piece_at[square];
    const PieceColor color = p.color();

    reset_ref_bb(occupied_bb, square);
    reset_ref_bb(colored_bb[color], square);
    reset_ref_bb(pieces_bb[color][p.type()], square);

    piece_at[square] = Piece(White, Empty);

    state.hash ^= Zobrist::pieces[color][square][p.type()];
}



#endif // POSITION_H
