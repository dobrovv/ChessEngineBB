#include "position.h"
#include <cstring> // memset()

Bitboard pawnCaptureStepsBB[COLOR_CNT][SQUARE_CNT];
Bitboard kingStepsBB[SQUARE_CNT];
Bitboard knightStepsBB[SQUARE_CNT];
Bitboard directionStepsBB[SQUARE_CNT][DIRECTION_CNT];
Direction fromToDirection[SQUARE_CNT][SQUARE_CNT];


Position::Position()
{
    occupied_bb = Empty_bb;
    std::memset(colored_bb, 0, sizeof(colored_bb) );
    std::memset(pieces_bb,  0, sizeof(pieces_bb) );
    std::memset(piece_at,    0, sizeof(piece_at) );

    state.castle_rights = NoCastlingFlags;
    state.epSquare = Square(a1);    // a1 == no En passant
    state.halfmove_clock = 0;
}

void Position::setPiece(Piece piece, Square square)
{
    assert(piece.type() != Empty);

    set_ref_bb(occupied_bb, square);
    set_ref_bb(colored_bb[piece.color()], square);
    set_ref_bb(pieces_bb[piece.color()][piece.type()], square);

    piece_at[square] = piece;
}

void Position::setPiece(PieceColor color, PieceType type, Square square)
{
    assert(type != Empty);

    set_ref_bb(occupied_bb, square);
    set_ref_bb(colored_bb[color], square);
    set_ref_bb(pieces_bb[color][type], square);

    piece_at[square] = Piece(color, type);
}

void Position::removePiece(Square square)
{
    assert(piece_at[square].type() != Empty);

    Piece p = piece_at[square];

    reset_ref_bb(occupied_bb, square);
    reset_ref_bb(colored_bb[p.color()], square);
    reset_ref_bb(pieces_bb[p.color()][p.type()], square);

    piece_at[square] = Piece(White, Empty);
}

void Position::movesForSide(PieceColor color, std::vector<Move> &moveList)
{

    if (color == White) {
        if (isKingAttacked<White>() ) {
            movesUnderCheck<White>(moveList);
        } else {
            movesForPawns<White>(moveList);
            movesForKnights<White>(moveList);
            movesForRayPieces<White, Bishop>(moveList);
            movesForRayPieces<White, Rook>(moveList);
            movesForRayPieces<White, Queen>(moveList);
            movesForKing<White>(moveList);
        }
    } else {
        if (isKingAttacked<Black>() ) {
            movesUnderCheck<Black>(moveList);
        } else {
            movesForPawns<Black>(moveList);
            movesForKnights<Black>(moveList);
            movesForRayPieces<Black, Bishop>(moveList);
            movesForRayPieces<Black, Rook>(moveList);
            movesForRayPieces<Black, Queen>(moveList);
            movesForKing<Black>(moveList);
        }
    }
}

template<PieceColor color>
void Position::movesUnderCheck(std::vector<Move>& moveList) {
    Bitboard kings = pieces(color, King);
    if (kings) {
        Square kingSquare = lsb_bb(kings);
        Bitboard attacker_set = attackersOf<!color>(kingSquare);
        assert(attacker_set != 0);

        if (popcount_bb(attacker_set) > 1) {
            return movesForKing<color>(moveList);   // time to panic
        }

        Square attackerSquare = lsb_bb(attacker_set);
        if (state.epSquare != NOT_ENPASSANT) {
            // check if we are attacked by a double pusshed pawn
            Square doublePawn = color == White
                    ? state.epSquare.prevRank()
                    : state.epSquare.nextRank();
            if (attackerSquare  == doublePawn){  // add en passnat capture
                Bitboard pawns = pawnCaptureStepsBB[!color][state.epSquare] & pieces(color, Pawn);
                while (pawns) {
                    Square pawn = pop_lsb_bb(pawns);
                    if (isAbsolutelyPinned<color>(pawn) == false) {
                        moveList.emplace_back(pawn, state.epSquare, CaptureEnPas);
                    }
                }
            }
        }

        Direction dir = fromToDirection[kingSquare][attackerSquare];
        Bitboard defend_ray = directionStepsBB[kingSquare][dir];
        while (defend_ray) {
            Bitboard target = pop_lsb_bb(defend_ray);
            movesToSquare<color>(target, moveList);
        }
    }
}

template<PieceColor color>
void Position::movesForPawns(std::vector<Move> &moveList)
{
    constexpr PieceColor Ally  = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    constexpr Direction  Forward   = color == White ? North : South;
    constexpr Direction  ForwardRight = color == White ? NorthEast : SouthWest;
    constexpr Direction  ForwardLeft  = color == White ? NorthWest : SouthEast;

    constexpr Bitboard   FirstPush_bb = color == White ? Rank3_bb : Rank6_bb;
    constexpr Bitboard   PromotionRank_bb = color == White ? Rank8_bb : Rank1_bb;

    const Bitboard notOccupied = ~occupied();
    const Bitboard allyPawns = pieces(Ally,Pawn);
    const Bitboard enemyPieces = colored(Enemy);

    Square origin;

    Bitboard pushed       = shift_bb<Forward>(allyPawns) & notOccupied;
    Bitboard doublePushed = shift_bb<Forward>(pushed & FirstPush_bb) & notOccupied;
    Bitboard promoted = pushed & PromotionRank_bb;
    pushed ^= promoted;

    foreach_pop_lsb(target, pushed) {
        origin = Ally == White ? target.prevRank() : target.nextRank();
        if (isAbsolutelyPinned<Ally>(origin, Forward) == false) {
            moveList.emplace_back(origin, target, QuietMove);
        }
    }

    foreach_pop_lsb(target, doublePushed) {
        origin = Ally == White ? target.prevRank().prevRank() : target.nextRank().nextRank();
        if (isAbsolutelyPinned<Ally>(origin, Forward) == false) {
            moveList.emplace_back(origin, target, DoublePush);
        }
    }

    foreach_pop_lsb(target, promoted) {
        origin = Ally == White ? target.prevRank() : target.nextRank();
        if (isAbsolutelyPinned<Ally>(origin, Forward) == false) {
            moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
            moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
        }
    }

    Bitboard rightCapture = shift_bb<ForwardRight>(allyPawns) & enemyPieces;
    Bitboard rightPromotion = rightCapture & PromotionRank_bb;
    rightCapture ^= rightPromotion;

    foreach_pop_lsb(target, rightCapture) {
        origin = Ally == White ? target.prevDiagMain() : target.nextDiagMain();
        if (isAbsolutelyPinned<Ally>(origin, ForwardRight) == false) {
            moveList.emplace_back(origin, target, Capture);
        }
    }

    foreach_pop_lsb(target, rightPromotion) {
        origin = Ally == White ? target.prevDiagMain() : target.nextDiagMain();
        if (isAbsolutelyPinned<Ally>(origin, ForwardRight) == false) {
            moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
            moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
        }
    }

    Bitboard leftCapture   = shift_bb<ForwardLeft>(allyPawns) & enemyPieces;
    Bitboard leftPromotion = leftCapture & PromotionRank_bb;
    leftCapture ^= leftPromotion;

    foreach_pop_lsb(target, leftCapture) {
        origin = Ally == White ? target.prevDiagAnti() : target.nextDiagAnti();
        if (isAbsolutelyPinned<Ally>(origin, ForwardLeft) == false) {
            moveList.emplace_back(origin, target, Capture);
        }
    }

    foreach_pop_lsb(target, leftPromotion) {
        origin = Ally == White ? target.prevDiagAnti() : target.nextDiagAnti();
        if (isAbsolutelyPinned<Ally>(origin, ForwardLeft) == false) {
            moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
            moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
        }
    }

    // En passant handling
    if (state.epSquare != NOT_ENPASSANT) {
        Bitboard pawns_set = pawnCaptureStepsBB[Enemy][state.epSquare] & allyPawns;
        foreach_pop_lsb(origin, pawns_set) {
            //if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][state.epSquare]) == false) {
            if (isEnPasCaptureLegal<Ally>(origin)) {
                moveList.emplace_back(origin, state.epSquare, CaptureEnPas);
            }
        }
    }
}

template<PieceColor color>
void Position::movesForKnights(std::vector<Move>& moveList)
{
    constexpr PieceColor Ally  = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    const Bitboard notOccupied = ~occupied();
    const Bitboard enemyPieces = colored(Enemy);

    Bitboard allyKnights = pieces(Ally, Knight);

    foreach_pop_lsb(origin, allyKnights) {
        if (isAbsolutelyPinned<Ally>(origin) )
            continue;

        const Bitboard stepsOn = knightStepsBB[origin];

        Bitboard quietMoves = stepsOn & notOccupied;
        foreach_pop_lsb(target, quietMoves)
            moveList.emplace_back(origin, target, QuietMove);

        Bitboard captureMoves = stepsOn & enemyPieces;
        foreach_pop_lsb(target, captureMoves)
            moveList.emplace_back(origin, target, Capture);
    }
}

template<PieceColor color, PieceType pieceType>
void Position::movesForRayPieces(std::vector<Move>& moveList) {

    assert(pieceType == Queen || pieceType == Rook || pieceType == Bishop);

    constexpr PieceColor Ally  = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    constexpr Direction DirectionStart = (pieceType == Bishop)
            ? NorthEast         // Start with diagolnal direction eg bishop
            : North;            // Start with orthogonal direction eg queen and rook

    constexpr std::uint8_t DirectionIncrement= (pieceType == Queen) // Rotate direction
                    ? 1                                          // by 45° for queen
                    : 2;                                         // by 90° for rook and bishop

    const Bitboard Occupied = occupied();
    const Bitboard enemyPieces = colored(Enemy);

    Bitboard allyRayPieces = pieces(Ally, pieceType);

    foreach_pop_lsb(origin, allyRayPieces) {

        for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement) ) {

            if (isAbsolutelyPinned<Ally>(origin, dir))
                continue;

            Bitboard stepsOn = rayPieceSteps(Occupied, origin, dir);
            Bitboard stepsOnPiece = stepsOn & Occupied;

            if (stepsOnPiece) {
                Square target = lsb_bb(stepsOnPiece);
                if (stepsOnPiece & enemyPieces)
                    moveList.emplace_back(origin, target, Capture);
                reset_ref_bb(stepsOn, target); // delete enemy piece/our piece from stepsOn
            }

            foreach_pop_lsb(target, stepsOn) {
                moveList.emplace_back(origin, target, QuietMove);
            }
        }
    }
}

template<PieceColor color>
void Position::movesForKing(std::vector<Move> &moveList)
{
    constexpr PieceColor Ally   = color == White ? White : Black;
    constexpr PieceColor Enemy  = color == White ? Black : White;

    // castling
    constexpr CastlingRights AllyCastleQSide = color == White ? CastlingFlagWQ : CastlingFlagBQ;
    constexpr CastlingRights AllyCastleKSide = color == White ? CastlingFlagWK : CastlingFlagBK;
    constexpr Square RookQSide   = color == White ? Square(a1) : Square(a8);
    constexpr Square RookKSide   = color == White ? Square(h1) : Square(h8);
    constexpr Square TargetQSide = color == White ? Square(c1) : Square(c8);
    constexpr Square TargetKSide = color == White ? Square(g1) : Square(g8);

    Bitboard allyKings = pieces(Ally, King);

    assert (allyKings != 0);

    const Bitboard Occupied = occupied();
    const Square origin = lsb_bb(allyKings);


    Bitboard stepsOn = kingStepsBB[origin];
    stepsOn ^= stepsOn & colored(Ally);    // delete same colored pieces

    Bitboard  captures = stepsOn & colored(Enemy);
    stepsOn ^= captures;   // delete captures subset

    foreach_pop_lsb(target, captures) {
        if (isAttackedBy<Enemy>(target) == false )
            moveList.emplace_back(origin, target, Capture);
    }

    foreach_pop_lsb(target, stepsOn) {
        if (isAttackedBy<Enemy>(target) == false )
            moveList.emplace_back(origin, target, QuietMove);
    }

    bool canCastleQSide =  state.castle_rights & AllyCastleQSide;
    if (canCastleQSide) {
        Bitboard kingStepsOn = rayPieceSteps(Occupied, RookQSide, East);
        if (popcount_bb(kingStepsOn) == 4) {

            pop_lsb_bb(kingStepsOn);   // unused - not on the kings way
            foreach_pop_lsb(square, kingStepsOn) {
                if (isAttackedBy<Enemy>(square) ) {
                    canCastleQSide = false; break;
                }
            }
            if (canCastleQSide)
                moveList.emplace_back(origin, TargetQSide, CastleQSide);
        }
    }

    bool canCastleKSide = state.castle_rights & AllyCastleKSide;
    if (canCastleKSide) {
        Bitboard kingStepsOn = rayPieceSteps(Occupied, RookKSide, West);
        if (popcount_bb(kingStepsOn) == 3) {

            foreach_pop_lsb(square, kingStepsOn) {
                if (isAttackedBy<Enemy>(square)) {
                    canCastleKSide = false;
                    break;
                }
            }
            if (canCastleKSide) {
                moveList.emplace_back(origin, TargetKSide, CastleKSide);
            }
        }
    }
}

// TODO: add castling as a way to reach the target square
template<PieceColor color>
void Position::movesToSquare(Square target, std::vector<Move>& moveList) {

    constexpr PieceColor Ally   = color == White ? White : Black;
    constexpr PieceColor Enemy  = color == White ? Black : White;

    const Bitboard allyPieces = colored(Ally);
    const Bitboard enemyPieces = colored(Enemy);
    const Bitboard allyPawns = pieces(Ally,Pawn);

    Bitboard seenBy = attackersOf<color>(target);

    if (is_set_bb(allyPieces, target))
        return; // nothing to do here then

    if (!is_set_bb(enemyPieces, target) && state.epSquare != target) { // target is not an opponent's piece nor the en passant square

        seenBy &= ~(seenBy & allyPawns); // remove pawn captures because target is not a piece

        //add a pawn that can move to the target square
        if (is_set_bb(pawnsPush<color>(), target)) {
            Square origin = color == White ? target.prevRank() : target.nextRank();
            set_ref_bb(seenBy, origin);
        }

        //add double pushed pawn that can move to target square
        if (is_set_bb(pawnsPushDouble<color>(), target)) {
            Square origin = color == White ? target.prevRank().prevRank() : target.nextRank().nextRank();
            //moveList.emplace_back(origin, target, DoublePush);
            set_ref_bb(seenBy, origin);
        }
    }

    while (seenBy) {
        Square origin = pop_lsb_bb(seenBy);
        Piece pieceOrig = pieceAt(origin);
        Piece pieceTrgt = pieceAt(target);

        if (pieceOrig.isPawn() ) { // is a pawn
            if (target == state.epSquare) {
                if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
                    moveList.emplace_back(origin, target, CaptureEnPas);
                }
            } else if ((color == White && target.rank() == 7) || (color == Black && target.rank() == 0)) {
                if ( pieceTrgt.isEmpty() ) {    // promote without capture
                    if (isAbsolutelyPinned<color>(origin, color == White ? North : South) == false) {
                        moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
                        moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
                    }
                } else {    // promote with capture
                    if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
                        moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
                        moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
                    }
                }
            } else {
                if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
                    moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture );
                }
            }
        } else if (pieceOrig.isKing() ) {
            if (isAttackedBy<!color>(target)) {
                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture );
            }
        } else { // knight, bishop, rook, queen
            if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture );
            }
        }
    }
}

// Singleton
struct GlobalsInitializer {
    GlobalsInitializer() {

        // Pawn steps array initialization
        for (Square origin = a1; origin <= h8; ++origin) {
            for(Direction dir = NorthEast; dir < DIRECTION_CNT; dir = Direction(dir+2)) {
                int file = directionStepOffsets[dir][0] + origin.file();
                int rank = directionStepOffsets[dir][1] + origin.rank();
                if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
                    if (dir == NorthEast || dir == NorthWest) { // white pawn
                        Square square(file,rank);
                        set_ref_bb(pawnCaptureStepsBB[White][origin], square);
                    } else if (dir == SouthEast || dir == SouthWest) { // black pawn
                        Square square(file,rank);
                        set_ref_bb(pawnCaptureStepsBB[Black][origin], square);
                    }
                }
            }
        }
        // Kings steps array initialization
        for (Square origin = a1; origin <= h8; ++origin) {
            Bitboard steps = 0;
            for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
                int file = directionStepOffsets[dir][0] + origin.file();
                int rank = directionStepOffsets[dir][1] + origin.rank();
                if (file >= 0 && file < 8 && rank >= 0 && rank < 8)
                    set_ref_bb(steps, Square(file, rank));
            }
            kingStepsBB[origin] = steps;
        }

        // Knight steps array initialization
        for (Square origin = a1; origin <= h8; ++origin) {
            Bitboard steps = 0;
            for (int ko=0; ko < 8; ++ko) {
                int file = knightStepOffsets[ko][0] + origin.file();
                int rank = knightStepOffsets[ko][1] + origin.rank();
                if (file >= 0 && file < 8 && rank >= 0 && rank < 8)
                    set_ref_bb(steps, Square(file,rank));
            }
            knightStepsBB[origin] = steps;
        }

        // Piece direction rays initialization
        for (Square origin = a1; origin < SQUARE_CNT; ++origin) {
            for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
                Square target = origin;
                Bitboard steps = 0;
                while(1) {
                    int file = directionStepOffsets[dir][0] + target.file();
                    int rank = directionStepOffsets[dir][1] + target.rank();
                    if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
                        target = Square(file,rank);
                        set_ref_bb(steps, target);
                    } else  {
                        break;
                    }
                }
                directionStepsBB[origin][dir] = steps;
            }
        }

        // Square direction initialization
        for (Square from = a1; from < SQUARE_CNT; ++from)
            for (Square to = a1; to < SQUARE_CNT; ++to)
                fromToDirection[from][to] = DIRECTION_CNT;

        for (Square from = a1; from < SQUARE_CNT; ++from) {
            for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
                for (Square to = a1; to < SQUARE_CNT; ++to) {
                    Bitboard ray = directionStepsBB[from][dir];
                    if (is_set_bb(ray, to))
                        fromToDirection[from][to] = dir;
                }
            }
        }
    }
} const globalsInitializer;



template<PieceColor color>
Bitboard Position::pawnsPush() {
    if (color == White) {
        return shift_bb<North>(pieces(White,Pawn)) & ~occupied();
    } else {
        return shift_bb<South>(pieces(Black,Pawn)) & ~occupied();
    }
}

template<PieceColor color>
Bitboard Position::pawnsPushDouble() {
    if (color == White) {
        Bitboard firstPush = shift_bb<North>(pieces(White,Pawn) & Rank2_bb) & ~occupied();
        return shift_bb<North>(firstPush) & ~occupied();
    } else {
        Bitboard firstPush = shift_bb<South>(pieces(Black,Pawn) & Rank7_bb) & ~occupied();
        return shift_bb<South>(firstPush) & ~occupied();
    }
}

template<PieceColor color>
Bitboard Position::pawnsCaptureEast() {
    if (color == White) {
        return shift_bb<NorthEast>(pieces(White,Pawn)) & colored(Black);
    } else {
        return shift_bb<SouthEast>(pieces(Black,Pawn)) & colored(White);
    }
}

template<PieceColor color>
Bitboard Position::pawnsCaptureWest() {
    if (color == White) {
        return shift_bb<NorthWest>(pieces(White,Pawn)) & colored(Black);
    } else {
        return shift_bb<SouthWest>(pieces(Black,Pawn)) & colored(White);
    }
}
