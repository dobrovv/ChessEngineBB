#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "bitboard.h"
#include "position.h"


/*--------------
 -- Move List --
 -------------*/

/* --
-- This is the main way moves are returned by the move generator, each element
-- consists of the move, and a priory score called sort used for move ordering during the search .
-- */

class MoveList {
    SortMove moveList[256];     // Maximum num of moves in chess is 218
    SortMove* end_of_list;

public:
    MoveList() {
        end_of_list = &moveList[0];
    }

    inline void emplace_back(Square origin, Square target, MoveType type) {
        *end_of_list++ = SortMove(origin, target, type);
    }

    inline bool empty() const {
        return end_of_list == &moveList[0];
    }

    inline size_t size() const {
        return end_of_list - &moveList[0];
    }

    inline SortMove& operator[](int index) { return moveList[index]; }

    SortMove* begin() { return &moveList[0]; }
    const SortMove* begin() const { return &moveList[0]; }
    SortMove* end() { return end_of_list; }
    const SortMove* end() const { return end_of_list; }
};

/*-----------------------------
 -- Move generation constans --
 ----------------------------*/

 // Directional offsets, file and rank offsets of a corresponding direction dir 
constexpr std::int8_t directionStepOffsets[DIRECTION_CNT][2] = { {-1,+1}, {+0,+1}, {+1,+1}, {+1,0}, {+1,-1}, {+0,-1}, {-1,-1}, {-1,+0} };

// Knight atacks, file and rank offset to the atacked squares
constexpr std::int8_t knightStepOffsets[8][2] = { {+1,+2}, {+2,+1}, {+2,-1}, {+1,-2}, {-1,-2}, {-2,-1}, {-2,+1}, {-1,+2} };

// Diagonal squares attacked by the pawn of color 'color' at square sq
extern Bitboard pawnCaptureStepsBB[COLOR_CNT][SQUARE_CNT];

// All adjacent and diagonal squares at square sq
extern Bitboard kingStepsBB[SQUARE_CNT];

// All squares attacked by a knight at square sq
extern Bitboard knightStepsBB[SQUARE_CNT];

// All ray squares starting at (and excluding) square sq in direction dir
extern Bitboard directionStepsBB[SQUARE_CNT][DIRECTION_CNT]; 

// Direction between suare sq and square sq2 otherwise NO_DIRECTION
extern Direction fromToDirection[SQUARE_CNT][SQUARE_CNT]; 


/*--------------------
 -- Move generation --
 -------------------*/

/*-- Generates all and only legal moves for the current stm --*/
inline void generate_all_moves(Position& pos, MoveList& moveList);

/* --
-- Finds the pinned pieces for the side Color and stores the pinned pieces in the position's pinned_bb variable
-- used by isAbsolutelyPinned() and must be computed for each new move before any move generation code
-- */
template<PieceColor Color>
void find_pinned_pieces(Position& pos);

/*----------------------------
 -- Move generation helpers --
 ---------------------------*/

template<PieceColor color>
void generate_pawn_moves(Position& pos, MoveList& moveList);

template<PieceColor color>
void generate_knight_moves(Position& pos, MoveList& moveList);

template<PieceColor color, PieceType pieceType>
void generate_sliding_piece_moves(Position& pos, MoveList& moveList);

template<PieceColor color>
void generate_king_moves(Position& pos, MoveList& moveList);

//template<PieceColor color>
//void generate_moves_to_target_sq(Position& pos, Square target, MoveList& moveList);

// same as generate_moves_to_target_sq but without king moves
// TODO: used for generate_moves_under_check, possibly redesign
template<PieceColor color>
void generate_nk_moves_to_target_sq(Position& pos, Square target, MoveList& moveList);

template<PieceColor color>
void generate_moves_under_check(Position& pos, MoveList& moveList);

/*-----------------------------
 -- Position state inqueries --
 ----------------------------*/

template <PieceColor bySide>
Bitboard attackersOf(const Position& pos, const Square square);

template<PieceColor attackerColor>
bool isAttackedBy(const Position& pos, const Square square);

template<PieceColor color>
bool isKingAttacked(const Position& pos);

template<PieceColor color>
bool isPinned(const Position& pos, const Square pinned, const Square to, const Direction ignored = NO_DIRECTION);

template<PieceColor color>
bool isAbsolutelyPinned(const Position& pos, const Square to, const Direction ignored = NO_DIRECTION);

template<PieceColor color>
bool isEnPasCaptureLegal(const Position& pos, const Square origin);

template<PieceColor color> inline Bitboard pawnsPush();
template<PieceColor color> inline Bitboard pawnsPushDouble();
template<PieceColor color> inline Bitboard pawnsCaptureEast();
template<PieceColor color> inline Bitboard pawnsCaptureWest();

inline bool isAttackedBy(const Position& pos, PieceColor attacker, const Square square) {
    if ( attacker == White )
        return isAttackedBy<White>(pos, square);
    else {
        return isAttackedBy<Black>(pos, square);
    }
}

inline bool isDefended( const Position& pos, PieceColor defender, const Square square) {
    if ( defender == White ) {
        return isAttackedBy<White>(pos, square);
    } else {
        return isAttackedBy<Black> (pos, square);
    }
}

/*-- Determines if the possition is currently in check --*/
inline bool IsKingInCheck(const Position& pos) {
    if ( pos.sideToMove() == White )
        return isKingAttacked<White>(pos);
    else
        return isKingAttacked<Black>(pos);
}

/*-- Generates all and only legal moves for the current stm --*/
inline void generate_all_moves(Position& pos, MoveList& moveList) {

    if (pos.sideToMove() == White) {

        find_pinned_pieces<White>(pos);

        if (isKingAttacked<White>(pos)) {
            generate_moves_under_check<White>(pos, moveList);
        } else {
            generate_pawn_moves<White>(pos, moveList);
            generate_knight_moves<White>(pos, moveList);
            generate_sliding_piece_moves<White, Bishop>(pos, moveList);
            generate_sliding_piece_moves<White, Rook>(pos, moveList);
            generate_sliding_piece_moves<White, Queen>(pos, moveList);
            generate_king_moves<White>(pos, moveList);
        }

    } else {

        find_pinned_pieces<Black>(pos);

        if (isKingAttacked<Black>(pos)) {
            generate_moves_under_check<Black>(pos, moveList);
        } else {
            generate_pawn_moves<Black>(pos, moveList);
            generate_knight_moves<Black>(pos, moveList);
            generate_sliding_piece_moves<Black, Bishop>(pos, moveList);
            generate_sliding_piece_moves<Black, Rook>(pos, moveList);
            generate_sliding_piece_moves<Black, Queen>(pos, moveList);
            generate_king_moves<Black>(pos, moveList);
        }
    }
}


template<PieceColor Color>
inline void find_pinned_pieces(Position& pos) {
    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    Bitboard pinned_bb = 0UL;
    Bitboard pinners = 0UL;

    Bitboard allyKings = pos.pieces(Ally, King);
    Bitboard enemyBishopsAndQueens = pos.pieces(Enemy, Bishop) | pos.pieces(Enemy, Queen);
    Bitboard enemyRooksAndQueens = pos.pieces(Enemy, Rook) | pos.pieces(Enemy, Queen);
    Bitboard Occupied = pos.occupied();

    assert(allyKings != 0);

    Square kingSquare = lsb_bb(allyKings);

    for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir + 1)) {

        // (1) find if an enemy bishop or queen or rook is on the directional ray
        
        if ( isDirectionRook(dir) ) {
            pinners = (directionStepsBB[kingSquare][dir] & enemyRooksAndQueens);
        } else if ( isDirectionBishop(dir) ) {
            pinners = (directionStepsBB[kingSquare][dir] & enemyBishopsAndQueens);
        }

        if (pinners) {
            // (2) get the closest pinner piece on the ray to the king
            Square pinnerPiece = isDirectionPositive(dir) ? lsb_bb(pinners) : msb_bb(pinners);

            // (3) find if there is a single pinned piece between the pinner and the king rays
            Bitboard pinnedPiece = rayPieceSteps(Occupied, kingSquare, dir) & rayPieceSteps(Occupied, pinnerPiece, invDir(dir)) & pos.colored(Ally);
            pinned_bb |= pinnedPiece;
        }
    }

    pos.setPinned(pinned_bb);

}

/*----------------------------
 -- Move generation helpers --
 ---------------------------*/

/* --
-- Sliding Piece Attacks
-- Details https://chessprogramming.wikispaces.com/Classical+Approach
-- Gets the directional attack of a piece up to (and including) the first blocker piece.
-- */
inline Bitboard rayPieceSteps(const Bitboard occupied, const Square origin, const Direction dir) {
    Bitboard ray = directionStepsBB[origin][dir];
    Bitboard blocker = ray & occupied;
    if (blocker) {
        Square blockerSquare = isDirectionPositive(dir)
            ? lsb_bb(blocker)      // direction is positive - scan forward
            : msb_bb(blocker);     // direction is nevatige - scan backward
        ray ^= directionStepsBB[blockerSquare][dir];
    }
    return ray;
}


template <PieceColor bySide>
inline Bitboard attackersOf(const Position& pos, const Square square) {

    constexpr PieceColor Attacker = bySide == White ? White : Black;
    constexpr PieceColor Defender = bySide == White ? Black : White;

    Bitboard result = 0;
    // check pawns
    result |= pawnCaptureStepsBB[Defender][square] & pos.pieces(Attacker, Pawn);
    // check knights
    result |= knightStepsBB[square] & pos.pieces(Attacker, Knight);
    // check kings
    result |= kingStepsBB[square] & pos.pieces(Attacker, King);

    const Bitboard Occupied = pos.occupied();
    const Bitboard attackerQueens = pos.pieces(Attacker, Queen);
    const Bitboard attackerRooks = pos.pieces(Attacker, Rook);
    const Bitboard attackerBishops = pos.pieces(Attacker, Bishop);
    const Bitboard attackerRayPieses = attackerQueens | attackerRooks | attackerBishops;

    // check ray pieces
    for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir + 1)) {
        Bitboard seenBy = rayPieceSteps(Occupied, square, dir) & attackerRayPieses;
        if (seenBy) {
            if (isDirectionRook(dir))
                result |= seenBy & (attackerQueens | attackerRooks);
            else
                result |= seenBy & (attackerQueens | attackerBishops);
        }

    }
    return result;
}

/* Returns true if the target square is attacked by the side attackerColor*/
template<PieceColor attackerColor>
inline bool isAttackedBy(const Position& pos, const Square target) {

    constexpr PieceColor Attacker = attackerColor == White ? White : Black;
    constexpr PieceColor Defender = attackerColor == White ? Black : White;

    // check pawns
    if (pawnCaptureStepsBB[Defender][target] & pos.pieces(Attacker, Pawn))
        return true;
    // check knights
    if (knightStepsBB[target] & pos.pieces(Attacker, Knight))
        return true;

    const Bitboard Occupied = pos.occupied();
    const Bitboard attackerQueens = pos.pieces(Attacker, Queen);
    const Bitboard attackerRooks = pos.pieces(Attacker, Rook);
    const Bitboard attackerBishops = pos.pieces(Attacker, Bishop);
    const Bitboard attackerRayPieses = attackerQueens | attackerRooks | attackerBishops;

    // check ray pieces
    for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir + 1)) {
        Bitboard seenBy = rayPieceSteps(Occupied, target, dir) & attackerRayPieses;

        if (isDirectionRook(dir)) {
            if (seenBy & (attackerQueens | attackerRooks))
                return true;
        }
        else {
            if (seenBy & (attackerQueens | attackerBishops))
                return true;
        }
    }

    if (kingStepsBB[target] & pos.pieces(Attacker, King))
        return true;

    return false;
}

template<PieceColor color>
inline bool isKingAttacked(const Position& pos) {
    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    Bitboard allyKings = pos.pieces(Ally, King);
    assert(allyKings != 0);
    Square kingSquare = lsb_bb(allyKings);

    return isAttackedBy<Enemy>(pos, kingSquare);
}

template<PieceColor color>
inline bool isPinned(const Position& pos, const Square pinned, const Square toPiece, const Direction ignored) {

    constexpr PieceColor Defender = color == White ? White : Black;
    constexpr PieceColor Attacker = color == White ? Black : White;

    Direction dir = fromToDirection[toPiece][pinned];

    // note the ignored parameter is used for pieces moving along the pinned line  TODO: (?) if pinned by multiple pieces
    // for now ignored direction should also include the opposite direction, for pieces that move back along the checked line
    if (dir == NO_DIRECTION || dir == ignored || (dir == ((ignored + 4) % DIRECTION_CNT) && ignored != NO_DIRECTION))
        return false;


    Bitboard occupied_set = pos.occupied();

    reset_ref_bb(occupied_set, pinned);

    //check if 'pinned' piece is obstructed on the path from toPiece
    Bitboard obstructed = (directionStepsBB[toPiece][dir] ^ directionStepsBB[pinned][dir]) & occupied_set;
    if (obstructed)
        return false;

    Bitboard ray = rayPieceSteps(occupied_set, toPiece, dir) & pos.colored(Attacker);
    if (ray) {
        Square attackerSquare = pop_lsb_bb(ray);
        Piece attacker = pos.pieceAt(attackerSquare);
        if (attacker.isQueen())
            return true;
        if (attacker.isRook() && isDirectionRook(dir))
            return true;
        if (attacker.isBishop() && isDirectionBishop(dir))
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
inline bool isAbsolutelyPinned(const Position& pos, const Square pinned, const Direction ignored) {
    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    Bitboard allyKings = pos.pieces(Ally, King);
    assert(allyKings != 0UL);
    Square kingSquare = lsb_bb(allyKings);

    // (1) find if the piece is in the pinned_bb set
    if (is_set_bb(pos.pinned(), pinned) == false)
        return false;

    // (2) check that the piece isn't moving along the pinned direction
    if (ignored != NO_DIRECTION && (ignored == fromToDirection[kingSquare][pinned] || ignored == fromToDirection[pinned][kingSquare]))
        return false;

    return true;

}

template<PieceColor color>
inline bool isEnPasCaptureLegal(const Position &pos, const Square origin) {
    assert(pos.pstate().epSquare != NOT_ENPASSANT);

    Bitboard kingBB = pos.pieces(color, King);
    if (kingBB) {
        // verify that the pawn is not pinned
        Square kingSquare = lsb_bb(kingBB);
        if (isAbsolutelyPinned<color>(pos, origin, fromToDirection[origin][pos.enPassantSq()]))
            return false;
        Square epPawnSquare = color == White
            ? pos.enPassantSq().prevRank()
            : pos.enPassantSq().nextRank();

        // En passant target square should be under attack of an opponent pawn for pseudo legality.
        // Further, for strict legality, the ep capturing pawn should not be absolutely pinned, 
        // which additionally requires a horizontal pin test of both involved pawns, which disappear from the same *rank*.

        // check if the king is located on the same rank as the pawns, that he is not pinned horizontaly
        Direction dir = fromToDirection[kingSquare][epPawnSquare];
        if (dir == East || dir == West) {
            Bitboard occupied_set = pos.occupied();
            reset_ref_bb(occupied_set, origin);
            reset_ref_bb(occupied_set, epPawnSquare);
            Bitboard ray = rayPieceSteps(occupied_set, kingSquare, dir) & pos.colored(!color) & occupied_set;
            if (ray) {
                Square attackerSquare = pop_lsb_bb(ray);
                Piece attacker = pos.pieceAt(attackerSquare);
                if (attacker.isQueen())
                    return false;
                if (attacker.isRook() && isDirectionRook(dir))
                    return false;
                if (attacker.isBishop() && isDirectionBishop(dir))
                    return false;
            }
        }
    }

    return true;
}

template<PieceColor color>
inline Bitboard pawnsPush(const Position& pos) {
    if (color == White) {
        return shift_bb<North>(pos.pieces(White, Pawn)) & ~pos.occupied();
    }
    else {
        return shift_bb<South>(pos.pieces(Black, Pawn)) & ~pos.occupied();
    }
}

template<PieceColor color>
inline Bitboard pawnsPushDouble(const Position& pos) {
    if (color == White) {
        Bitboard firstPush = shift_bb<North>(pos.pieces(White, Pawn) & Rank2_bb) & ~pos.occupied();
        return shift_bb<North>(firstPush) & ~pos.occupied();
    }
    else {
        Bitboard firstPush = shift_bb<South>(pos.pieces(Black, Pawn) & Rank7_bb) & ~pos.occupied();
        return shift_bb<South>(firstPush) & ~pos.occupied();
    }
}

template<PieceColor color>
inline Bitboard pawnsCaptureEast(const Position& pos) {
    if (color == White) {
        return shift_bb<NorthEast>(pos.pieces(White, Pawn)) & pos.colored(Black);
    }
    else {
        return shift_bb<SouthEast>(pos.pieces(Black, Pawn)) & pos.colored(White);
    }
}

template<PieceColor color>
inline Bitboard pawnsCaptureWest(const Position& pos) {
    if (color == White) {
        return shift_bb<NorthWest>(pos.pieces(White, Pawn)) & pos.colored(Black);
    }
    else {
        return shift_bb<SouthWest>(pos.pieces(Black, Pawn)) & pos.colored(White);
    }
}



#endif