#ifndef POSITION_H
#define POSITION_H

#include "types.h"
#include "bitboard.h"

extern Bitboard pawnCaptureStepsBB[COLOR_CNT][SQUARE_CNT];
extern Bitboard kingStepsBB[SQUARE_CNT];
extern Bitboard knightStepsBB[SQUARE_CNT];
extern Bitboard directionStepsBB[SQUARE_CNT][DIRECTION_CNT];
extern Direction fromToDirection[SQUARE_CNT][SQUARE_CNT];

enum CastlingRights : std::uint8_t {
    NoCastlingFlags = 0,
    CastlingFlagWQ = SHL(1,0),
    CastlingFlagWK = SHL(1,1),
    CastlingFlagBQ = SHL(1,2),
    CastlingFlagBK = SHL(1,3)
};

const Square NOT_ENPASSANT(a1); // invalid En Passant square

struct PositionState {
    std::uint8_t castle_rights;
    Square epSquare;            // En Passant square, a1 == invalid
    std::uint8_t halfmove_clock; // half move clock;
};


class Position
{

protected:
    Bitboard occupied_bb;
    Bitboard colored_bb[COLOR_CNT];
    Bitboard pieces_bb[COLOR_CNT][TYPE_CNT];

    Piece  piece_at[SQUARE_CNT];

    PositionState state;
    PieceColor side;      // player to move next

public:
    Position();

    Piece pieceAt(Square square)   const { return piece_at[square]; }
    void setPiece(Piece piece, Square square);
    void setPiece(PieceColor color, PieceType type, Square square);
    void removePiece(Square square);    

    inline Bitboard occupied() const { return occupied_bb; }
    inline Bitboard colored(PieceColor withColor)            const { return colored_bb[withColor]; }
    inline Bitboard pieces(PieceColor color, PieceType type) const { return pieces_bb[color][type]; }

    template <PieceColor bySide>
    Bitboard attackers(Square square);

    inline PieceColor sideToMove() const { return side;}
    inline void setSideToMove(PieceColor color) { side = color; }

    inline PositionState positionState() const { return state; }
    inline void setPositionState(PositionState positionState) { state = positionState;}

    //bool isOccupied(Square square) const { return is_set_bb(occupied_bb, square); }

    template<PieceColor bySide>
    bool isAttacked(Square square);

    template<PieceColor color>
    bool isKingAttacked();

    template<PieceColor color>
    bool isPinned(Square pinned, Square to, Direction ignored = DIRECTION_CNT);

    template<PieceColor color>
    bool isAbsolutelyPinned(Square to, Direction ignored = DIRECTION_CNT);

    void movesForSide(PieceColor color, std::vector<Move>& moveList);
    //void movesForPawns(PieceColor color, std::vector<Move> &moveList);

private:    /* helper functions */
    template<PieceColor color>
    void movesForPawns(std::vector<Move>& moveList);

    template<PieceColor color>
    void movesForKnights(std::vector<Move>& moveList);

    template<PieceColor color, PieceType pieceType>
    void movesForRayPieces(std::vector<Move>& moveList);

    template<PieceColor color>
    void movesForKing(std::vector<Move>& moveList);

    template<PieceColor color>
    void movesToSquare(Square target, std::vector<Move>& moveList);

    template<PieceColor color>
    void movesUnderCheck(std::vector<Move> &moveList);

    template<PieceColor color> Bitboard pawnsPush();
    template<PieceColor color> Bitboard pawnsPushDouble();
    template<PieceColor color> Bitboard pawnsCaptureEast();
    template<PieceColor color> Bitboard pawnsCaptureWest();

};

// Sliding Piece Attacks source:
// https://chessprogramming.wikispaces.com/Classical+Approach
inline Bitboard rayPieceSteps(Bitboard occupied, Square origin, Direction dir) {
    Bitboard ray = directionStepsBB[origin][dir];
    Bitboard blocker = ray & occupied;
    if (blocker) {
        Square blockerSquare = (dir < 3 || dir == 7)
                ? bitscan_forward(blocker)      // direction is positive - scan forward
                : bitscan_reverse(blocker);     // direction is nevatige - scan reverse
        ray ^= directionStepsBB[blockerSquare][dir];
    }
    return ray;
}

template <PieceColor bySide>
Bitboard Position::attackers(Square square) {
    Bitboard result = 0;
    // check pawns
    result |= pawnCaptureStepsBB[!bySide][square] & pieces(bySide, Pawn);
    // check knights
    result |= knightStepsBB[square] & pieces(bySide, Knight);
    // check kings
    result |= kingStepsBB[square] & pieces(bySide, King);

    // check ray pieces
    for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
        Bitboard attacker_one = rayPieceSteps(occupied(), square, dir) & colored(bySide);
        if (attacker_one) {
            if (dir == North || dir == East || dir == South || dir == West)
                result |= attacker_one & (pieces(bySide, Rook) | pieces(bySide, Queen));
            else
                result |= attacker_one & (pieces(bySide, Bishop) | pieces(bySide, Queen));
        }

    }
    return result;
}

template<PieceColor bySide>
bool Position::isAttacked(Square square) {
    // check pawns
    if (pawnCaptureStepsBB[!bySide][square] & pieces(bySide, Pawn))
        return true;
    // check knights
    if (knightStepsBB[square] & pieces(bySide, Knight) )
        return true;
    // check ray pieces
    for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
        Bitboard attacker_one = rayPieceSteps(occupied(), square, dir) & colored(bySide);
        if (attacker_one) {
            if (dir == North || dir == East || dir == South || dir == West) {
                if (attacker_one & (pieces(bySide, Rook) | pieces(bySide, Queen) ))
                        return true;
            } else {
                if (attacker_one & (pieces(bySide, Bishop) | pieces(bySide, Queen) ))
                    return true;
            }
        }
    }
    if (kingStepsBB[square] & pieces(bySide, King))
        return true;
    return false;
}

template<PieceColor color>
bool Position::isKingAttacked() {
    Bitboard kings = pieces(color, King);
    if (kings) {
        Square kingSquare = bitscan_forward(kings);
        return isAttacked<!color>(kingSquare);
    }
    return false;
}

template<PieceColor color>
bool Position::isPinned(Square pinned, Square to, Direction ignored) {
    Direction dir = fromToDirection[to][pinned];
    if (dir == DIRECTION_CNT || dir == ignored)
        return false;

    Bitboard occupied_set = occupied();
    reset_ref_bb(occupied_set, pinned);
    Bitboard ray = rayPieceSteps(occupied_set, to, dir) & colored(!color);
    if (ray) {
        Square attackerSquare = pop_lsb(ray);
        Piece attacker = pieceAt(attackerSquare);
        if (attacker.isQueen())
            return true;
        if (attacker.isRook() && (dir == North || dir == East || dir == South || dir == West) )
            return true;
        if (attacker.isBishop() && (dir == NorthEast || dir == SouthEast || dir == SouthWest || dir == NorthWest) )
            return true;
    }
    return false;
}

template<PieceColor color>
bool Position::isAbsolutelyPinned(Square pinned, Direction ignored) {
    Bitboard kings = pieces(color, King);
    if (kings) {
        Square kingSquare = bitscan_forward(kings);
        return isPinned<color>(pinned, kingSquare, ignored);
    }
    return false;
}

#endif // POSITION_H
