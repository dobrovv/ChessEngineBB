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
    Bitboard attackersOf(Square square);

    inline PieceColor sideToMove() const { return side;}
    inline void setSideToMove(PieceColor color) { side = color; }

    inline PositionState positionState() const { return state; }
    inline void setPositionState(PositionState positionState) { state = positionState;}

    //bool isOccupied(Square square) const { return is_set_bb(occupied_bb, square); }

    template<PieceColor color>
    bool isAttackedBy(Square square);

    template<PieceColor color>
    bool isKingAttacked();

    template<PieceColor color>
    bool isPinned(Square pinned, Square to, Direction ignored = DIRECTION_CNT);

    template<PieceColor color>
    bool isAbsolutelyPinned(Square to, Direction ignored = DIRECTION_CNT);

    template<PieceColor color>
    bool isEnPasCaptureLegal(Square origin);

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
                ? lsb_bb(blocker)      // direction is positive - scan forward
                : msb_bb(blocker);     // direction is nevatige - scan reverse
        ray ^= directionStepsBB[blockerSquare][dir];
    }
    return ray;
}

template <PieceColor bySide>
Bitboard Position::attackersOf(Square square) {

    constexpr PieceColor Attacker = bySide == White ? White : Black;
    constexpr PieceColor Defender = bySide == White ? Black : White;

    Bitboard result = 0;
    // check pawns
    result |= pawnCaptureStepsBB[Defender][square] & pieces(Attacker, Pawn);
    // check knights
    result |= knightStepsBB[square] & pieces(Attacker, Knight);
    // check kings
    result |= kingStepsBB[square] & pieces(Attacker, King);

    const Bitboard Occupied = occupied();
    const Bitboard attackerQueens  = pieces(Attacker, Queen);
    const Bitboard attackerRooks   = pieces(Attacker, Rook);
    const Bitboard attackerBishops = pieces(Attacker, Bishop);
    const Bitboard attackerRayPieses = attackerQueens | attackerRooks | attackerBishops;

    // check ray pieces
    for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
        Bitboard seenBy = rayPieceSteps(Occupied, square, dir) & attackerRayPieses;
        if (seenBy) {
            if (dir == North || dir == East || dir == South || dir == West)
                result |= seenBy & (attackerQueens | attackerRooks);
            else
                result |= seenBy & (attackerQueens | attackerBishops);
        }

    }
    return result;
}

template<PieceColor color>
bool Position::isAttackedBy(Square square) {

    constexpr PieceColor Attacker = color == White ? White : Black;
    constexpr PieceColor Defender = color == White ? Black : White;

    // check pawns
    if (pawnCaptureStepsBB[Defender][square] & pieces(Attacker, Pawn))
        return true;
    // check knights
    if (knightStepsBB[square] & pieces(Attacker, Knight) )
        return true;

    const Bitboard Occupied = occupied();
    const Bitboard attackerQueens  = pieces(Attacker, Queen);
    const Bitboard attackerRooks   = pieces(Attacker, Rook);
    const Bitboard attackerBishops = pieces(Attacker, Bishop);
    const Bitboard attackerRayPieses = attackerQueens | attackerRooks | attackerBishops;

    // check ray pieces
    for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
        Bitboard seenBy = rayPieceSteps(Occupied, square, dir) & attackerRayPieses;
        if (seenBy) {
            if (dir == North || dir == East || dir == South || dir == West) {
                if (seenBy & (attackerQueens | attackerRooks))
                        return true;
            } else {
                if (seenBy & (attackerQueens | attackerBishops))
                    return true;
            }
        }
    }

    if (kingStepsBB[square] & pieces(Attacker, King))
        return true;

    return false;
}

template<PieceColor color>
bool Position::isKingAttacked() {
    constexpr PieceColor Ally  = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    Bitboard allyKings = pieces(Ally, King);
    assert(allyKings != 0);
    Square kingSquare = lsb_bb(allyKings);

    return isAttackedBy<Enemy>(kingSquare);
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
        Square attackerSquare = pop_lsb_bb(ray);
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
    Bitboard kingBB = pieces(color, King);
    if (kingBB) {
        Square kingSquare = lsb_bb(kingBB);
        return isPinned<color>(pinned, kingSquare, ignored);
    }
    return false;
}

template<PieceColor color>
bool Position::isEnPasCaptureLegal(Square origin) {
    assert(state.epSquare != NOT_ENPASSANT);

    Bitboard kingBB = pieces(color, King);
    if (kingBB) {
        Square kingSquare = lsb_bb(kingBB);
        if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][state.epSquare]))
            return false;
        Square epPawnSquare = color == White
                ? state.epSquare.prevRank()
                : state.epSquare.nextRank();

        Direction dir = fromToDirection[kingSquare][epPawnSquare];
        if (dir != DIRECTION_CNT) {
            Bitboard occupied_set = occupied();
            reset_ref_bb(occupied_set, origin);
            reset_ref_bb(occupied_set, epPawnSquare);
            Bitboard ray = rayPieceSteps(occupied_set, kingSquare, dir) & colored(!color) & occupied_set;
            if (ray) {
                Square attackerSquare = pop_lsb_bb(ray);
                Piece attacker = pieceAt(attackerSquare);
                if (attacker.isQueen())
                    return false;
                if (attacker.isRook() && (dir == North || dir == East || dir == South || dir == West) )
                    return false;
                if (attacker.isBishop() && (dir == NorthEast || dir == SouthEast || dir == SouthWest || dir == NorthWest) )
                    return false;
            }
        }
    }

    return true;
}


#endif // POSITION_H
