#ifndef EVALUATE_H
#define EVALUATE_H

#include "position.h"
#include "movegen.h"

//r1bqkb1r/pp2nppp/2p5/3p3Q/P3n3/2N1P3/1PP2PPP/R1B1KBNR w KQ - 2 15


/*---------------
 -- Evaluation --
 --------------*/

 // Piece Weights for material calucaltion in centipawns
 // Details for weights https://www.chessprogramming.org/Simplified_Evaluation_Function
const static int PieceWeight[TYPE_CNT] = {
    0,       // Empty
    100,     // Pawn
    320,     // Knight,
    330,     // Bishop, 
    500,     // Rook, 
    900,     // Queen
    0,       // King
};


template <PieceColor Color> inline uint32_t mobility_score_for(const Position& pos);
template <PieceColor Color> inline uint32_t mobility_for_knight(const Position& pos, Square origin);
template <PieceColor Color> inline uint32_t mobility_for_bishop(const Position& pos, Square origin);
template <PieceColor Color> inline uint32_t mobility_for_rook(const Position& pos, Square origin);
template <PieceColor Color> inline uint32_t mobility_for_queen(const Position& pos, Square origin);

// Helper, Finds all blocked pawns for the side Color
template <PieceColor Color> inline Bitboard blockedPawns(const Position& pos);


inline int32_t evaluate(Position& pos) {


    // Weight of one extra square reached by a piece for mobility calculation
    const static int MobilityWeight = 1;

    int32_t materialScore = 0;

    for (PieceType pt = Pawn; pt < King; pt = PieceType(pt + 1)) {
        materialScore += PieceWeight[pt] * (popcount_bb(pos.pieces(White, pt)) - popcount_bb(pos.pieces(Black, pt)));
    }

    find_pinned_pieces<White>(pos);
    int32_t whiteMobility = mobility_score_for<White>(pos);

    find_pinned_pieces<Black>(pos);
    int32_t blackMobility = mobility_score_for<Black>(pos);

    int32_t mobilityScore = MobilityWeight * (whiteMobility - blackMobility);

    return materialScore + mobilityScore;
}


template <PieceColor Color>
inline uint32_t mobility_score_for(const Position&  pos) {
    constexpr PieceColor Ally = Color == White ? White : Black;

    uint32_t mobility = 0;

    Bitboard allyKnights = pos.pieces(Ally, Knight);
    Bitboard allyBishops = pos.pieces(Ally, Bishop);
    Bitboard allyRooks = pos.pieces(Ally, Rook);
    Bitboard allyQueens = pos.pieces(Ally, Queen);

    foreach_pop_lsb(origin, allyKnights) {
        mobility += mobility_for_knight<Ally>(pos, origin);
    }

    foreach_pop_lsb(origin, allyBishops) {
        mobility += mobility_for_bishop<Ally>(pos, origin);
    }

    foreach_pop_lsb(origin, allyRooks) {
        mobility += mobility_for_rook<Ally>(pos, origin);
    }

    foreach_pop_lsb(origin, allyQueens) {
        mobility += mobility_for_queen<Ally>(pos, origin);
    }

    return mobility;
}

template <PieceColor Color>
inline uint32_t mobility_for_knight(const Position& pos, Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // Knights can attack our knight, queen, bishops and rooks for mobility.
    // Knights only take into account enemy pawns, not queen, rooks, bishops, knights, king etc

    assert(pos.pieceAt(origin).type() == Knight);

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = pos.occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>(pos) | pawnsCaptureWest<Enemy>(pos);
    const Bitboard BlockedPawns = blockedPawns<Ally>(pos);


    uint32_t mobility = 0;

    if (isAbsolutelyPinned<Ally>(pos, origin, NO_DIRECTION))
        return mobility;

    const Bitboard excluded =
        (pos.pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pos.pieces(Ally, King) |                    // our king
        BlockedPawns |                              // blocked pawns
        enemyPawnAttacks;                           // attacked by enemy pawn

    Bitboard stepsOn = knightStepsBB[origin];

    // Knights can attack our knight, queen, bishops and rooks for mobility.
    // Knights can attack their own pieces for mobility calculation(TODO: except pawns blocked and in rank 2 and 3 and our king)
    stepsOn &= ~excluded;

    mobility += popcount_bb(stepsOn);


    return mobility;
}

template <PieceColor Color>
inline uint32_t mobility_for_bishop(const Position& pos, Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation (except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // Bishop can attack our knight, bishop and rooks for mobility.
    // Bishop does not consider attack from enemy queen or rooks or bishop or knight or king.
    // Bishops can look through our queen.

    assert(pos.pieceAt(origin).type() == Bishop);

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Direction DirectionStart = NorthWest;     // Start with diagolnal x-direction eg queen and bishop
    constexpr std::uint8_t DirectionIncrement = 2;      // rotate by 90� for rook and bishop

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = pos.occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>(pos) | pawnsCaptureWest<Enemy>(pos);
    const Bitboard BlockedPawns = blockedPawns<Ally>(pos);

    // Bishops can look throught our queen
    const Bitboard OccupiedNoQueen = Occupied ^ pos.pieces(Ally, Queen);

    uint32_t mobility = 0;

    const Bitboard excluded =
        (pos.pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pos.pieces(Ally, King) |                    // our king
        BlockedPawns |                              // blocked pawns
        enemyPawnAttacks;                           // attacked by enemy pawn

    for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

        if (isAbsolutelyPinned<Ally>(pos, origin, dir))
            continue;

        Bitboard stepsOn = rayPieceSteps(OccupiedNoQueen, origin, dir);

        // Bishops consider enemy piece attack as mobility area.
        // Bishop can attack our knight, bishop and rooks for mobility.
        // Bishop can attack their own pieces for mobility calculation
        stepsOn &= ~excluded;

        mobility += popcount_bb(stepsOn);
    }

    return mobility;
}

template <PieceColor Color>
inline uint32_t mobility_for_rook(const Position& pos, Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // Rooks can look through our queen and rook.
    // Rook does not consider attack from enemy queen or rooks or bishop or knight or king.
    
    assert(pos.pieceAt(origin).type() == Rook);

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Direction DirectionStart = North;     // Start with vertival direction eg rook
    constexpr std::uint8_t DirectionIncrement = 2;  // rotate by 90� for rook and bishop

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = pos.occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>(pos) | pawnsCaptureWest<Enemy>(pos);
    const Bitboard BlockedPawns = blockedPawns<Ally>(pos);

    // Rooks can look through our queenand rook
    const Bitboard OccupiedNoQueenAndRook = Occupied ^ (pos.pieces(Ally, Queen) | pos.pieces(Ally, Rook));

    uint32_t mobility = 0;

    const Bitboard excluded =
        (pos.pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pos.pieces(Ally, King) |                    // our king
        BlockedPawns |                              // blocked pawns
        enemyPawnAttacks;                           // attacked by enemy pawn

    for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

        if (isAbsolutelyPinned<Ally>(pos, origin, dir))
            continue;

        // Rooks can look through our queenand rook
        Bitboard stepsOn = rayPieceSteps(OccupiedNoQueenAndRook, origin, dir);

        // Rooks consider enemy piece attack as mobility area.
        // Rooks can attack our knight, bishop and rooks for mobility.
        // Rooks can attack their own pieces for mobility calculation
        stepsOn &= ~excluded;

        mobility += popcount_bb(stepsOn);
    }

    return mobility;
}

template <PieceColor Color>
inline uint32_t mobility_for_queen(const Position& pos, Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // TODO: Queen takes into account attacks from enemy bishop, knight and rook. Not enemy queen or king.
    // Queen cannot look through our rook or bishop.

    assert(pos.pieceAt(origin).type() == Queen);

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Direction DirectionStart = NorthWest;   // Start with diagolnal x-direction eg queen and bishop
    constexpr std::uint8_t DirectionIncrement = 1;    // rotate by 45� for queen

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = pos.occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>(pos) | pawnsCaptureWest<Enemy>(pos);
    const Bitboard BlockedPawns = blockedPawns<Ally>(pos);

    uint32_t mobility = 0;

    const Bitboard excluded =
        (pos.pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pos.pieces(Ally, King) |                    // our king
        BlockedPawns |                              // blocked pawns
        enemyPawnAttacks;                           // attacked by enemy pawn

    // TODO: Queen takes into account attacks from enemy bishop, knight and rook. Not enemy queen or king.

    for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

        if (isAbsolutelyPinned<Ally>(pos, origin, dir))
            continue;

        // Rooks can look through our queenand rook
        Bitboard stepsOn = rayPieceSteps(Occupied, origin, dir);

        // Queens consider enemy piece attack as mobility area.
        // Queens can attack our knight, bishop and rooks for mobility.
        // Queens can attack their own pieces for mobility calculation
        stepsOn &= ~excluded;

        mobility += popcount_bb(stepsOn);
    }

    return mobility;
}

// Returns bitboard of blocked pawns for the side Color
template <PieceColor Color>
inline Bitboard blockedPawns(const Position& pos) {
    
    constexpr PieceColor Ally    = Color == White ? White : Black;
    constexpr Direction Forward  = Color == White ? North : South;
    constexpr Direction Backward = Color == White ? South : North;
    

    Bitboard pawns_blocked_step_forward = shift_bb<Forward>(pos.pieces(Ally, Pawn)) & pos.occupied();
    return shift_bb<Backward>(pawns_blocked_step_forward);
}

#endif