#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"
#include "position.h"
#include <vector>
#include <string> // for FEN

class Board : public Position
{
public:
//private:
    std::vector<Move>  moves_done;
    std::vector<PositionState> state_stack;
    std::vector<Piece> captured_pieses;


public:
    Board();
    void moveDo(Move move);
    void moveUndo();

    static Board fromFEN(std::string fenRecord);
    
    static Board startpos();

    template <PieceColor Color>
    uint32_t getMobilityScore();

    template <PieceColor Color>
    inline uint32_t getMobilityForKnight(Square origin);

    template <PieceColor Color>
    inline uint32_t getMobilityForBishop(Square origin);

    template <PieceColor Color>
    inline uint32_t getMobilityForRook(Square origin);

    template <PieceColor Color>
    inline uint32_t getMobilityForQueen(Square origin);

    //TODO:s
    // Evaluate the current position and return value in centipawns
    int32_t evaluate();

    //TODO:s 
    // Search for the best move
    uint64_t search(ExtMove& result, int depth = 5);

    uint64_t searchDo(int depth, int& bestValue, Move& bestMove);

    uint64_t searchAB(ExtMove& result, int depth = 5);

    uint64_t searchDoAB(int depth, int& bestValue, Move& bestMove, int alpha, int beta, bool isMaxTurn);


};

template <PieceColor Color>
uint32_t Board::getMobilityScore() {
    constexpr PieceColor Ally = Color == White ? White : Black;

    uint32_t mobility = 0;

    Bitboard allyKnights = pieces(Ally, Knight);
    Bitboard allyBishops = pieces(Ally, Bishop);
    Bitboard allyRooks   = pieces(Ally, Rook);
    Bitboard allyQueens = pieces(Ally, Queen);

    foreach_pop_lsb(origin, allyKnights) {
        mobility += getMobilityForKnight<Ally>(origin);
    }

    foreach_pop_lsb(origin, allyBishops) {
        mobility += getMobilityForBishop<Ally>(origin);
    }

    foreach_pop_lsb(origin, allyRooks) {
        mobility += getMobilityForRook<Ally>(origin);
    }

    foreach_pop_lsb(origin, allyQueens) {
        mobility += getMobilityForQueen<Ally>(origin);
    }

    return mobility;
}

template <PieceColor Color>
inline uint32_t Board::getMobilityForKnight(Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // Knights can attack our knight, queen, bishops and rooks for mobility.
    // Knights only take into account enemy pawns, not queen, rooks, bishops, knights, king etc

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>() | pawnsCaptureWest<Enemy>();


    uint32_t mobility = 0;

    if (isAbsolutelyPinned<Ally>(origin))
        return mobility;

    const Bitboard excluded =
        (pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pieces(Ally, King) |                    // our king
        //                                      // ? blocked pawns
        enemyPawnAttacks;                       // attacked by enemy pawn

    Bitboard stepsOn = knightStepsBB[origin];

    // Knights can attack our knight, queen, bishops and rooks for mobility.
    // Knights can attack their own pieces for mobility calculation(TODO: except pawns blocked and in rank 2 and 3 and our king)
    stepsOn &= ~excluded;

    mobility += popcount_bb(stepsOn);


    return mobility;
}

template <PieceColor Color>
uint32_t Board::getMobilityForBishop(Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // Bishop can attack our knight, bishop and rooks for mobility.
    // Bishop does not consider attack from enemy queen or rooks or bishop or knight or king.
    // Bishops can look through our queen.

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Direction DirectionStart = NorthWest;     // Start with diagolnal x-direction eg queen and bishop
    constexpr std::uint8_t DirectionIncrement = 2;      // rotate by 90� for rook and bishop

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>() | pawnsCaptureWest<Enemy>();

    // Bishops can look throught our queen
    const Bitboard OccupiedNoQueen = Occupied ^ pieces(Ally, Queen);

    uint32_t mobility = 0;

    const Bitboard excluded =
        (pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pieces(Ally, King) |                    // our king
        //                                      // ? blocked pawns
        enemyPawnAttacks;                       // attacked by enemy pawn

    for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

        if (isAbsolutelyPinned<Ally>(origin, dir))
            continue;

        Bitboard stepsOn = rayPieceSteps(OccupiedNoQueen, origin, dir);

        // Bishops consider enemy piece attack as mobility area.
        // Bishop can attack our knight, bishop and rooks for mobility.
        // Bishop can attack their own pieces for mobility calculation( except TODO: pawns blocked and in rank 2 and 3 and our king)
        stepsOn &= ~excluded;

        mobility += popcount_bb(stepsOn);
    }

    return mobility;
}

template <PieceColor Color>
uint32_t Board::getMobilityForRook(Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // Rooks can look through our queen and rook.
    // Rook does not consider attack from enemy queen or rooks or bishop or knight or king.

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Direction DirectionStart = North;     // Start with vertival direction eg rook
    constexpr std::uint8_t DirectionIncrement = 2;  // rotate by 90� for rook and bishop

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>() | pawnsCaptureWest<Enemy>();

    // Rooks can look through our queenand rook
    const Bitboard OccupiedNoQueenAndRook = Occupied ^ (pieces(Ally, Queen) | pieces(Ally, Rook));

    uint32_t mobility = 0;

    const Bitboard excluded =
        (pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pieces(Ally, King) |                    // our king
        //                                      // ? blocked pawns
        enemyPawnAttacks;                       // attacked by enemy pawn

    for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

        if (isAbsolutelyPinned<Ally>(origin, dir))
            continue;
        
        // Rooks can look through our queenand rook
        Bitboard stepsOn = rayPieceSteps(OccupiedNoQueenAndRook, origin, dir);

        // Rooks consider enemy piece attack as mobility area.
        // Rooks can attack our knight, bishop and rooks for mobility.
        // Rooks can attack their own pieces for mobility calculation( except pawns TODO: blocked and in rank 2 and 3 and our king)
        stepsOn &= ~excluded;

        mobility += popcount_bb(stepsOn);
    }

    return mobility;
}

template <PieceColor Color>
uint32_t Board::getMobilityForQueen(Square origin) {
    // Our pawns in rank 2 and 3, blocked pawns and our king are excluded from mobility area.
    // All pieces consider enemy piece attack as mobility area.
    // All can attack their own pieces for mobility calculation(except pawns blocked and in rank 2 and 3 and our king)
    // All pieces do not take squares attacked by enemy pawn in the mobility area.
    // TODO: Queen takes into account attacks from enemy bishop, knight and rook. Not enemy queen or king.
    // Queen cannot look through our rook or bishop.

    constexpr PieceColor Ally = Color == White ? White : Black;
    constexpr PieceColor Enemy = Color == White ? Black : White;

    constexpr Direction DirectionStart = NorthWest;   // Start with diagolnal x-direction eg queen and bishop
    constexpr std::uint8_t DirectionIncrement = 1;    // rotate by 45� for queen

    constexpr Bitboard Ranks2and3 = Color == White ? (Rank2_bb | Rank3_bb) : (Rank7_bb | Rank6_bb);

    const Bitboard Occupied = occupied();
    const Bitboard enemyPawnAttacks = pawnsCaptureEast<Enemy>() | pawnsCaptureWest<Enemy>();

    uint32_t mobility = 0;

    const Bitboard excluded =
        (pieces(Ally, Pawn) & (Ranks2and3)) |   // Our pawns in rank 2 and 3
        pieces(Ally, King) |                    // our king
        //                                      // ? blocked pawns
        enemyPawnAttacks;                       // attacked by enemy pawn

    // TODO: Queen takes into account attacks from enemy bishop, knight and rook. Not enemy queen or king.

    for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

        if (isAbsolutelyPinned<Ally>(origin, dir))
            continue;

        // Rooks can look through our queenand rook
        Bitboard stepsOn = rayPieceSteps(Occupied, origin, dir);

        // Queens consider enemy piece attack as mobility area.
        // Queens can attack our knight, bishop and rooks for mobility.
        // Queens can attack their own pieces for mobility calculation( except pawns TODO: blocked and in rank 2 and 3 and our king)
        stepsOn &= ~excluded;

        mobility += popcount_bb(stepsOn);
    }

    return mobility;
}



#endif // BOARD_H
