#ifndef POSITION_H
#define POSITION_H

#include "types.h"
#include "bitboard.h"

//  directional offsets, file and rank offsets of a corresponding direction dir 
constexpr std::int8_t directionStepOffsets[DIRECTION_CNT][2] = { {-1,+1}, {+0,+1}, {+1,+1}, {+1,0}, {+1,-1}, {+0,-1}, {-1,-1}, {-1,+0} };

//  knight atacks, file and rank offset to the atacked squares
constexpr std::int8_t knightStepOffsets[8][2] = { {+1,+2}, {+2,+1}, {+2,-1}, {+1,-2}, {-1,-2}, {-2,-1}, {-2,+1}, {-1,+2} };

extern Bitboard pawnCaptureStepsBB[COLOR_CNT][SQUARE_CNT]; // diagonal squares attacked by the pawn of color 'color' at square sq
extern Bitboard kingStepsBB[SQUARE_CNT]; // all adjacent and diagonal squares at square sq
extern Bitboard knightStepsBB[SQUARE_CNT]; //all squares attacked by a knight at square sq
extern Bitboard directionStepsBB[SQUARE_CNT][DIRECTION_CNT]; // all ray squares starting at (and excluding) square sq in direction dir
extern Direction fromToDirection[SQUARE_CNT][SQUARE_CNT]; // direction between suare sq and square sq2 or NO_DIRECTION

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
public:
//protected:
    Bitboard occupied_bb;
    Bitboard colored_bb[COLOR_CNT];
    Bitboard pieces_bb[COLOR_CNT][TYPE_CNT];

    Piece  piece_at[SQUARE_CNT];

    PositionState state;
    PieceColor side;      // sideToMove next player to move

    Bitboard pinned_bb;

public:
    Position();

    inline Piece pieceAt(Square square)   const { return piece_at[square]; }
    inline void setPiece(Piece piece, Square square);
    inline void setPiece(PieceColor color, PieceType type, Square square);
    inline void removePiece(Square square);    

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
    bool isPinned(Square pinned, Square to, Direction ignored = NO_DIRECTION);
    
    template<PieceColor color>
    bool isAbsolutelyPinned(Square to, Direction ignored = NO_DIRECTION);

    template<PieceColor color>
    bool isEnPasCaptureLegal(Square origin);

    void movesForSide(PieceColor color, std::vector<Move>& moveList);
    //void movesForPawns(PieceColor color, std::vector<Move> &moveList);

private:    /* helper functions */

protected:
    // finds the pinned pieces for the side Color and stores the pinned pieces in pinned_bb
    template<PieceColor Color>
    void getPinnedPieces();

private:

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
    void movesToSquareNoKing(Square target, std::vector<Move>& moveList);

    template<PieceColor color>
    void movesUnderCheck(std::vector<Move> &moveList);

protected:
    template<PieceColor color> inline Bitboard pawnsPush();
    template<PieceColor color> inline Bitboard pawnsPushDouble();
    template<PieceColor color> inline Bitboard pawnsCaptureEast();
    template<PieceColor color> inline Bitboard pawnsCaptureWest();

};

inline void Position::setPiece(Piece piece, Square square)
{
    assert(piece.type() != Empty);

    const PieceColor color = piece.color();

    set_ref_bb(occupied_bb, square);
    set_ref_bb(colored_bb[color], square);
    set_ref_bb(pieces_bb[color][piece.type()], square);

    piece_at[square] = piece;
}

inline void Position::setPiece(PieceColor color, PieceType type, Square square)
{
    assert(type != Empty);

    set_ref_bb(occupied_bb, square);
    set_ref_bb(colored_bb[color], square);
    set_ref_bb(pieces_bb[color][type], square);

    piece_at[square] = Piece(color, type);
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
}

// Sliding Piece Attacks source:
// https://chessprogramming.wikispaces.com/Classical+Approach
// gets the directional attack of a piece up to (and including) the first blocker piece.
inline Bitboard rayPieceSteps(Bitboard occupied, Square origin, Direction dir) {
    Bitboard ray = directionStepsBB[origin][dir];
    Bitboard blocker = ray & occupied;
    if (blocker) {
        Square blockerSquare = isDirectionPositive(dir)
                ? lsb_bb(blocker)      // direction is positive - scan forward
                : msb_bb(blocker);     // direction is nevatige - scan reverse
        ray ^= directionStepsBB[blockerSquare][dir];
    }
    return ray;
}

template <PieceColor bySide>
inline Bitboard Position::attackersOf(Square square) {

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
    for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
        Bitboard seenBy = rayPieceSteps(Occupied, square, dir) & attackerRayPieses;
        if (seenBy) {
            if ( isDirectionRook(dir) )
                result |= seenBy & (attackerQueens | attackerRooks);
            else
                result |= seenBy & (attackerQueens | attackerBishops);
        }

    }
    return result;
}

/* Returns true if the square target is attacked by the side attackerColor*/
template<PieceColor attackerColor>
inline bool Position::isAttackedBy(Square target) {

    constexpr PieceColor Attacker = attackerColor == White ? White : Black;
    constexpr PieceColor Defender = attackerColor == White ? Black : White;

    // check pawns
    if (pawnCaptureStepsBB[Defender][target] & pieces(Attacker, Pawn))
        return true;
    // check knights
    if (knightStepsBB[target] & pieces(Attacker, Knight) )
        return true;

    const Bitboard Occupied = occupied();
    const Bitboard attackerQueens  = pieces(Attacker, Queen);
    const Bitboard attackerRooks   = pieces(Attacker, Rook);
    const Bitboard attackerBishops = pieces(Attacker, Bishop);
    const Bitboard attackerRayPieses = attackerQueens | attackerRooks | attackerBishops;

    // check ray pieces
    for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
        Bitboard seenBy = rayPieceSteps(Occupied, target, dir) & attackerRayPieses;
        
        if (isDirectionRook(dir)) {
            if (seenBy & (attackerQueens | attackerRooks))
                    return true;
        } else {
            if (seenBy & (attackerQueens | attackerBishops))
                return true;
        }
    }

    if (kingStepsBB[target] & pieces(Attacker, King))
        return true;

    return false;
}

template<PieceColor color>
inline bool Position::isKingAttacked() {
    constexpr PieceColor Ally  = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    Bitboard allyKings = pieces(Ally, King);
    assert(allyKings != 0);
    Square kingSquare = lsb_bb(allyKings);

    return isAttackedBy<Enemy>(kingSquare);
}

template<PieceColor color>
inline bool Position::isPinned(Square pinned, Square toPiece, Direction ignored) {

    constexpr PieceColor Defender = color == White ? White : Black;
    constexpr PieceColor Attacker = color == White ? Black : White;

    Direction dir = fromToDirection[toPiece][pinned];
    
    // note the ignored parameter is used for pieces moving along the pinned line  TODO: (?) if pinned by multiple pieces
    // for now ignored direction should also include the opposite direction, for pieces that move back along the checked line
    if (dir == NO_DIRECTION || dir == ignored || (dir == ((ignored+4) % DIRECTION_CNT) && ignored != NO_DIRECTION) )
        return false;


    Bitboard occupied_set = occupied();

    reset_ref_bb(occupied_set, pinned);
    
    //check if 'pinned' piece is obstructed on the path from toPiece
    Bitboard obstructed = (directionStepsBB[toPiece][dir] ^ directionStepsBB[pinned][dir]) & occupied_set;
    if (obstructed)
        return false;

    Bitboard ray = rayPieceSteps(occupied_set, toPiece, dir) & colored(Attacker);
    if (ray) {
        Square attackerSquare = pop_lsb_bb(ray);
        Piece attacker = pieceAt(attackerSquare);
        if (attacker.isQueen())
            return true;
        if (attacker.isRook() && isDirectionRook(dir) )
            return true;
        if (attacker.isBishop() && isDirectionBishop(dir) )
            return true;
    }
    return false;
}

/*
template<PieceColor color>
inline bool Position::isAbsolutelyPinned(Square pinned, Direction ignored) {
    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    Bitboard allyKings = pieces(Ally, King);
    assert(allyKings != 0UL);
    Square kingSquare = lsb_bb(allyKings);

    return isPinned<color>(pinned, kingSquare, ignored);
}
*/

template<PieceColor Color>
inline bool Position::isAbsolutelyPinned(Square pinned, Direction ignored) {
    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    Bitboard allyKings = pieces(Ally, King);
    assert(allyKings != 0UL);
    Square kingSquare = lsb_bb(allyKings);

    // (1) find if the piece is in the pinned_bb set
    if (is_set_bb(pinned_bb, pinned) == false)
        return false;
    
    // (2) check that the piece isn't moving along the pinned direction
    if (ignored != NO_DIRECTION && (ignored == fromToDirection[kingSquare][pinned] || ignored == fromToDirection[pinned][kingSquare]))
        return false;

    return true;

}

template<PieceColor color>
inline bool Position::isEnPasCaptureLegal(Square origin) {
    assert(state.epSquare != NOT_ENPASSANT);

    Bitboard kingBB = pieces(color, King);
    if (kingBB) {
        // verify that the pawn is not pinned
        Square kingSquare = lsb_bb(kingBB);
        if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][state.epSquare]))
            return false;
        Square epPawnSquare = color == White
                ? state.epSquare.prevRank()
                : state.epSquare.nextRank();

        //En passant target square should be under attack of an opponent pawn for pseudo legality.
        //Further, for strict legality, the ep capturing pawn should not be absolutely pinned, 
        //which additionally requires a horizontal pin test of both involved pawns, which disappear from the same *rank*.

        // check if the king is located on the same rank as the pawns, that he is not pinned horizontaly
        Direction dir = fromToDirection[kingSquare][epPawnSquare];
        if (dir == East || dir == West) {
            Bitboard occupied_set = occupied();
            reset_ref_bb(occupied_set, origin);
            reset_ref_bb(occupied_set, epPawnSquare);
            Bitboard ray = rayPieceSteps(occupied_set, kingSquare, dir) & colored(!color) & occupied_set;
            if (ray) {  
                Square attackerSquare = pop_lsb_bb(ray);
                Piece attacker = pieceAt(attackerSquare);
                if ( attacker.isQueen() )
                    return false;
                if ( attacker.isRook() && isDirectionRook(dir) )
                    return false;
                if ( attacker.isBishop() && isDirectionBishop(dir) )
                    return false;
            }
        }
    }

    return true;
}

template<PieceColor color>
inline Bitboard Position::pawnsPush() {
    if (color == White) {
        return shift_bb<North>(pieces(White, Pawn)) & ~occupied();
    }
    else {
        return shift_bb<South>(pieces(Black, Pawn)) & ~occupied();
    }
}

template<PieceColor color>
inline Bitboard Position::pawnsPushDouble() {
    if (color == White) {
        Bitboard firstPush = shift_bb<North>(pieces(White, Pawn) & Rank2_bb) & ~occupied();
        return shift_bb<North>(firstPush) & ~occupied();
    }
    else {
        Bitboard firstPush = shift_bb<South>(pieces(Black, Pawn) & Rank7_bb) & ~occupied();
        return shift_bb<South>(firstPush) & ~occupied();
    }
}

template<PieceColor color>
inline Bitboard Position::pawnsCaptureEast() {
    if (color == White) {
        return shift_bb<NorthEast>(pieces(White, Pawn)) & colored(Black);
    }
    else {
        return shift_bb<SouthEast>(pieces(Black, Pawn)) & colored(White);
    }
}

template<PieceColor color>
inline Bitboard Position::pawnsCaptureWest() {
    if (color == White) {
        return shift_bb<NorthWest>(pieces(White, Pawn)) & colored(Black);
    }
    else {
        return shift_bb<SouthWest>(pieces(Black, Pawn)) & colored(White);
    }
}


#endif // POSITION_H
