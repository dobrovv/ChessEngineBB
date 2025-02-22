#include "movegen.h"

/* --
 -- Main move g
 -- Generates only legal moves
 -- */
inline void generate_all_moves(Position& pos, MoveList& moveList);

/*----------------------------
 -- Move generation helpers --
 ---------------------------*/

template<PieceColor color>
void generate_pawn_moves(Position& pos, MoveList& moveList)
{
    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    constexpr Direction  Forward = color == White ? North : South;
    constexpr Direction  ForwardRight = color == White ? NorthEast : SouthWest;
    constexpr Direction  ForwardLeft = color == White ? NorthWest : SouthEast;

    constexpr Bitboard   FirstPush_bb = color == White ? Rank3_bb : Rank6_bb;
    constexpr Bitboard   PromotionRank_bb = color == White ? Rank8_bb : Rank1_bb;

    const Bitboard notOccupied = ~pos.occupied();
    const Bitboard allyPawns = pos.pieces(Ally, Pawn);
    const Bitboard enemyPieces = pos.colored(Enemy);

    Square origin;

    Bitboard pushed = shift_bb<Forward>(allyPawns) & notOccupied;
    Bitboard doublePushed = shift_bb<Forward>(pushed & FirstPush_bb) & notOccupied;
    Bitboard promoted = pushed & PromotionRank_bb;
    pushed ^= promoted;

    foreach_pop_lsb(target, pushed) {
        origin = Ally == White ? target.prevRank() : target.nextRank();
        if (isAbsolutelyPinned<Ally>(pos, origin, Forward) == false) {
            moveList.emplace_back(origin, target, QuietMove);
        }
    }

    foreach_pop_lsb(target, doublePushed) {
        origin = Ally == White ? target.prevRank().prevRank() : target.nextRank().nextRank();
        if (isAbsolutelyPinned<Ally>(pos, origin, Forward) == false) {
            moveList.emplace_back(origin, target, DoublePush);
        }
    }

    foreach_pop_lsb(target, promoted) {
        origin = Ally == White ? target.prevRank() : target.nextRank();
        if (isAbsolutelyPinned<Ally>(pos, origin, Forward) == false) {
            moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
            moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
        }
    }

    Bitboard rightCapture = shift_bb<ForwardRight>(allyPawns) & enemyPieces;
    Bitboard rightPromotion = rightCapture & PromotionRank_bb;
    rightCapture ^= rightPromotion;

    foreach_pop_lsb(target, rightCapture) {
        origin = Ally == White ? target.prevDiagMain() : target.nextDiagMain();
        if (isAbsolutelyPinned<Ally>(pos, origin, ForwardRight) == false) {
            moveList.emplace_back(origin, target, Capture);
        }
    }

    foreach_pop_lsb(target, rightPromotion) {
        origin = Ally == White ? target.prevDiagMain() : target.nextDiagMain();
        if (isAbsolutelyPinned<Ally>(pos, origin, ForwardRight) == false) {
            moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
            moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
        }
    }

    Bitboard leftCapture = shift_bb<ForwardLeft>(allyPawns) & enemyPieces;
    Bitboard leftPromotion = leftCapture & PromotionRank_bb;
    leftCapture ^= leftPromotion;

    foreach_pop_lsb(target, leftCapture) {
        origin = Ally == White ? target.prevDiagAnti() : target.nextDiagAnti();
        if (isAbsolutelyPinned<Ally>(pos, origin, ForwardLeft) == false) {
            moveList.emplace_back(origin, target, Capture);
        }
    }

    foreach_pop_lsb(target, leftPromotion) {
        origin = Ally == White ? target.prevDiagAnti() : target.nextDiagAnti();
        if (isAbsolutelyPinned<Ally>(pos, origin, ForwardLeft) == false) {
            moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
            moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
        }
    }

    // En passant handling
    if (pos.enPassantSq() != NOT_ENPASSANT) {
        Bitboard pawns_set = pawnCaptureStepsBB[Enemy][pos.enPassantSq()] & allyPawns;
        foreach_pop_lsb(origin, pawns_set) {
            //if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][state.epSquare]) == false) {
            if (isEnPasCaptureLegal<Ally>(pos, origin)) {
                moveList.emplace_back(origin, pos.enPassantSq(), CaptureEnPas);
            }
        }
    }
}

template<PieceColor color>
void generate_knight_moves(Position& pos, MoveList& moveList)
{
    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    const Bitboard notOccupied = ~pos.occupied();
    const Bitboard enemyPieces = pos.colored(Enemy);

    Bitboard allyKnights = pos.pieces(Ally, Knight);

    foreach_pop_lsb(origin, allyKnights) {
        if (isAbsolutelyPinned<Ally>(pos, origin))
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
void generate_sliding_piece_moves(Position& pos, MoveList& moveList) {

    static_assert(pieceType == Queen || pieceType == Rook || pieceType == Bishop);

    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    constexpr Direction DirectionStart = (pieceType == Rook)
        ? North         // Start with orthogonal +-direction eg rook
        : NorthWest;    // Start with diagolnal x-direction eg queen and bishop

    constexpr std::uint8_t DirectionIncrement = (pieceType == Queen) // Rotate direction
        ? 1                                          // by 45° for queen
        : 2;                                         // by 90° for rook and bishop

    const Bitboard Occupied = pos.occupied();
    const Bitboard enemyPieces = pos.colored(Enemy);

    Bitboard allyRayPieces = pos.pieces(Ally, pieceType);

    foreach_pop_lsb(origin, allyRayPieces) {

        for (Direction dir = DirectionStart; dir < DIRECTION_CNT; dir = Direction(dir + DirectionIncrement)) {

            if (isAbsolutelyPinned<Ally>(pos, origin, dir))
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

/*-- Helper to detect if a piece is attacked by a sliding piece from a direction --*/
template<PieceColor enemyColor>
inline bool isAttackedFromRay(Position &pos, Square origin, Direction dir) {
    
    const Bitboard Occupied = pos.occupied();

    if ( isRookDirection(dir) ) {
        const Bitboard attackerQueens = pos.pieces(enemyColor, Queen);
        const Bitboard attackerRooks = pos.pieces(enemyColor, Rook);
        const Bitboard seenBy = rayPieceSteps(Occupied, origin, dir) & (attackerQueens | attackerRooks);

        if ( seenBy )
            return true;

    } else {
        const Bitboard attackerQueens = pos.pieces(enemyColor, Queen);
        const Bitboard attackerBishops = pos.pieces(enemyColor, Bishop);
        const Bitboard seenBy = rayPieceSteps(Occupied, origin, dir) & (attackerQueens | attackerBishops);

        if ( seenBy )
            return true;
    }

    return false;
}

template<PieceColor color>
void generate_king_moves(Position& pos, MoveList& moveList)
{
    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    // castling
    constexpr CastlingRights AllyCastleQSide = color == White ? CastlingFlagWQ : CastlingFlagBQ;
    constexpr CastlingRights AllyCastleKSide = color == White ? CastlingFlagWK : CastlingFlagBK;
    constexpr Square RookQSide = color == White ? Square(a1) : Square(a8);
    constexpr Square RookKSide = color == White ? Square(h1) : Square(h8);
    constexpr Square TargetQSide = color == White ? Square(c1) : Square(c8);
    constexpr Square TargetKSide = color == White ? Square(g1) : Square(g8);

    Bitboard allyKings = pos.pieces(Ally, King);

    assert(allyKings != 0);

    const Bitboard Occupied = pos.occupied();
    const Square kingSq = lsb_bb(allyKings);


    Bitboard stepsOn = kingStepsBB[kingSq];
    stepsOn ^= stepsOn & pos.colored(Ally);    // delete square with our pieces

    Bitboard  captures = stepsOn & pos.colored(Enemy);
    stepsOn ^= captures;   // delete the subset of captures

    foreach_pop_lsb(target, captures) {
        if ( isAttackedFromRay<Enemy>(pos, kingSq, fromToDirection[target][kingSq]) == false 
            && isAttackedBy<Enemy>(pos, target) == false ) {
            moveList.emplace_back(kingSq, target, Capture);
        }
    }

    foreach_pop_lsb(target, stepsOn) {
        if ( isAttackedFromRay<Enemy>(pos, kingSq, fromToDirection[target][kingSq]) == false
            && isAttackedBy<Enemy>(pos, target) == false ) {
            moveList.emplace_back(kingSq, target, QuietMove);
        }
    }

    // Note: Rook moves and castle rights are handled by moveDo/moveUndo, but check that the rook is on it's original square i.e. rook isn't captured by an enemy piece.
    bool canCastleQSide = (pos.castling() & AllyCastleQSide) && (pos.pieces(Ally, Rook) & SHL(1, RookQSide));
    if (canCastleQSide) {
        Bitboard kingStepsOn = rayPieceSteps(Occupied, RookQSide, East);
        if (popcount_bb(kingStepsOn) == 4) {

            pop_lsb_bb(kingStepsOn);   // unused - not on the kings way
            foreach_pop_lsb(square, kingStepsOn) {
                if (isAttackedBy<Enemy>(pos, square)) {
                    canCastleQSide = false; 
                    break;
                }
            }
            if (canCastleQSide)
                moveList.emplace_back(kingSq, TargetQSide, CastleQSide);
        }
    }

    bool canCastleKSide = (pos.castling() & AllyCastleKSide) && (pos.pieces(Ally, Rook) & SHL(1, RookKSide));
    if (canCastleKSide) {
        Bitboard kingStepsOn = rayPieceSteps(Occupied, RookKSide, West);
        if (popcount_bb(kingStepsOn) == 3) {

            //pop_msb_bb(kingStepsOn);   // unused - not on the kings way
            foreach_pop_lsb(square, kingStepsOn) {
                if (isAttackedBy<Enemy>(pos, square)) {
                    canCastleKSide = false;
                    break;
                }
            }
            if (canCastleKSide) {
                moveList.emplace_back(kingSq, TargetKSide, CastleKSide);
            }
        }
    }
}

// TODO: add castling as a way to reach the target square, (movesToSquare currently not used)
//template<PieceColor color>
//void generate_moves_to_target(Position& pos, Square target, std::vector<Move>& moveList) {
//
//    constexpr PieceColor Ally = color == White ? White : Black;
//    constexpr PieceColor Enemy = color == White ? Black : White;
//
//    const Bitboard allyPieces = colored(Ally);
//    const Bitboard enemyPieces = colored(Enemy);
//    const Bitboard allyPawns = pieces(Ally, Pawn);
//
//    if (is_set_bb(allyPieces, target))
//        return; // nothing to do here then
//
//    Bitboard seenBy = attackersOf<Ally>(target);
//
//
//    if (!is_set_bb(enemyPieces, target) && state.epSquare != target) { // target is not an opponent's piece nor the en passant square
//
//        seenBy &= ~(seenBy & allyPawns); // remove pawn captures because target is not a piece
//
//        //add the pawn that can move to the target square
//        if (is_set_bb(pawnsPush<color>(), target)) {
//            Square origin = color == White ? target.prevRank() : target.nextRank();
//            set_ref_bb(seenBy, origin);
//        }
//
//        //add double pushed pawn that can move to target square
//        if (is_set_bb(pawnsPushDouble<color>(), target)) {
//            Square origin = color == White ? target.prevRank().prevRank() : target.nextRank().nextRank();
//            //moveList.emplace_back(origin, target, DoublePush);
//            set_ref_bb(seenBy, origin);
//        }
//    }
//
//    while (seenBy) {
//        Square origin = pop_lsb_bb(seenBy);
//        Piece pieceOrig = pieceAt(origin);
//        Piece pieceTrgt = pieceAt(target);
//
//        if (pieceOrig.isPawn()) { // is a pawn
//            if (target == state.epSquare) {
//                if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
//                    moveList.emplace_back(origin, target, CaptureEnPas);
//                }
//            }
//            else if ((color == White && target.rank() == 7) || (color == Black && target.rank() == 0)) {
//                if (pieceTrgt.isEmpty()) {    // promote without capture
//                    if (isAbsolutelyPinned<color>(origin, color == White ? North : South) == false) {
//                        moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
//                        moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
//                    }
//                }
//                else {    // promote with capture
//                    if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
//                        moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
//                        moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
//                    }
//                }
//            }
//            else {
//                if (isAbsolutelyPinned<color>(origin, fromToDirection[origin][target]) == false) {
//
//                    bool isDoublePushedPawn = color == White ? (target - 16) == origin : (target + 16) == origin;
//
//                    if (isDoublePushedPawn) {
//                        moveList.emplace_back(origin, target, DoublePush);
//                    }
//                    else {
//                        moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture);
//                    }
//                }
//            }
//        }
//        else if (pieceOrig.isKing()) {
//            if (isAttackedBy<Enemy>(target) == false) {
//                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture);
//            }
//        }
//        else { // knight, bishop, rook, queen
//            if (isAbsolutelyPinned<Ally>(origin, fromToDirection[origin][target]) == false) {
//                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture);
//            }
//        }
//    }
//}

// same as generate_moves_to_target_sq but without king moves used for generate_moves_under_check
template<PieceColor color>
void generate_nk_moves_to_target_sq(Position& pos, Square target, MoveList& moveList) {

    constexpr PieceColor Ally = color == White ? White : Black;
    constexpr PieceColor Enemy = color == White ? Black : White;

    const Bitboard allyPieces = pos.colored(Ally);
    const Bitboard enemyPieces = pos.colored(Enemy);
    const Bitboard allyPawns = pos.pieces(Ally, Pawn);

    if (is_set_bb(allyPieces, target))
        return; // nothing to do here then

    Bitboard seenBy = attackersOf<Ally>(pos, target);


    if (!is_set_bb(enemyPieces, target) && pos.enPassantSq() != target) { // target is not an opponent's piece nor the en passant square

        seenBy &= ~(seenBy & allyPawns); // remove pawn captures because target is not a piece

        //add the pawn that can move to the target square
        if (is_set_bb(pawnsPush<color>(pos), target)) {
            Square origin = color == White ? target.prevRank() : target.nextRank();
            set_ref_bb(seenBy, origin);
        }

        //add double pushed pawn that can move to target square
        if (is_set_bb(pawnsPushDouble<color>(pos), target)) {
            Square origin = color == White ? target.prevRank().prevRank() : target.nextRank().nextRank();
            //moveList.emplace_back(origin, target, DoublePush);
            set_ref_bb(seenBy, origin);
        }
    }

    while (seenBy) {
        Square origin = pop_lsb_bb(seenBy);
        Piece pieceOrig = pos.pieceAt(origin);
        Piece pieceTrgt = pos.pieceAt(target);

        if (pieceOrig.isPawn()) { // is a pawn
            if (target == pos.enPassantSq()) {
                if (isAbsolutelyPinned<color>(pos, origin, fromToDirection[origin][target]) == false) {
                    moveList.emplace_back(origin, target, CaptureEnPas);
                }
            }
            else if ((color == White && target.rank() == 7) || (color == Black && target.rank() == 0)) {
                if (pieceTrgt.isEmpty()) {    // promote without capture
                    if (isAbsolutelyPinned<color>(pos, origin, color == White ? North : South) == false) {
                        moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
                        moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
                    }
                }
                else {    // promote with capture
                    if (isAbsolutelyPinned<color>(pos, origin, fromToDirection[origin][target]) == false) {
                        moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
                        moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
                    }
                }
            }
            else {
                if (isAbsolutelyPinned<color>(pos, origin, fromToDirection[origin][target]) == false) {

                    bool isDoublePushedPawn = color == White ? (target - 16) == origin : (target + 16) == origin;

                    if (isDoublePushedPawn) {
                        moveList.emplace_back(origin, target, DoublePush);
                    }
                    else {
                        moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture);
                    }
                }
            }
        }
        else if (pieceOrig.isKing()) {
            // king moves are ignored
            /*if (isAttackedBy<Enemy>(target) == false) {
                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture);
            }*/
        }
        else { // knight, bishop, rook, queen
            if (isAbsolutelyPinned<Ally>(pos, origin, fromToDirection[origin][target]) == false) {
                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture);
            }
        }
    }
}

template<PieceColor color>
void generate_moves_under_check(Position& pos, MoveList& moveList) {
    constexpr PieceColor Defender = color == White ? White : Black;
    constexpr PieceColor Attacker = color == White ? Black : White;

    Bitboard kings = pos.pieces(Defender, King);

    assert(kings != 0);

    Square kingSquare = lsb_bb(kings);
    Bitboard attacker_set = attackersOf<Attacker>(pos, kingSquare);
    assert(attacker_set != 0);

    // get king moves, NOTE: since the defend_ray direction can't contain king moves the set of moves is disjoint with the generate_nk_moves_to_target_sq() for the king piece
    // TODO: the check for the king moves on defend_ray is currently done multiple times
    generate_king_moves<Defender>(pos, moveList);

    // get moves for pieces to the ray between the king and the attacker
    if (popcount_bb(attacker_set) > 1) {
        /*The king must move since there is no way to block two different lines or take two different pieces at the same time*/
        return;   // time to panic
    }

    Square attackerSquare = lsb_bb(attacker_set);
    if (pos.enPassantSq() != NOT_ENPASSANT) {
        // check if we are attacked by a double pushed pawn
        Square doublePawn = color == White
            ? pos.enPassantSq().prevRank()
            : pos.enPassantSq().nextRank();
        if (attackerSquare == doublePawn) {  // add en passnat capture
            Bitboard pawns = pawnCaptureStepsBB[Attacker][pos.enPassantSq()] & pos.pieces(color, Pawn);
            while (pawns) {
                Square pawn = pop_lsb_bb(pawns);
                if (isAbsolutelyPinned<Defender>(pos, pawn) == false) {
                    moveList.emplace_back(pawn, pos.enPassantSq(), CaptureEnPas);
                }
            }
        }
    }

    //Direction dir = fromToDirection[kingSquare][attackerSquare];
    //Bitboard defend_ray = directionStepsBB[kingSquare][dir];



    // set the attacker's square as the target of the move
    // this is the only case if the attacker is a knight or a pawn
    Bitboard defend_ray = attacker_set;

    /* Handle addtion of attackers that are ray pieces, queens, bishops and rooks*/
    if ((pos.pieces(Attacker, Knight) & attacker_set) == 0UL) {
        Direction dir = fromToDirection[attackerSquare][kingSquare];
        defend_ray |= rayPieceSteps(pos.occupied(), attackerSquare, dir); // add ray segment from attacker to target, (excluding attacker's square)
    }

    while (defend_ray) {
        Bitboard target = pop_lsb_bb(defend_ray);
        // since generate_king_moves() handles the case when the attacker is on an adjacent square to the king, use generate_nk_moves_to_target_sq()
        generate_nk_moves_to_target_sq<color>(pos, target, moveList);
    }
}


/*-- singleton to initialize bitboards that are used for move generation --*/
struct GlobalsInitializerMoves {
    GlobalsInitializerMoves() {

        // Pawn steps array initialization
        for (Square origin = a1; origin <= h8; ++origin) {
            for (Direction dir = NorthWest; dir < DIRECTION_CNT; dir = Direction(dir + 2)) {
                int file = directionStepOffsets[dir][0] + origin.file();
                int rank = directionStepOffsets[dir][1] + origin.rank();
                if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
                    if (dir == NorthWest || dir == NorthEast) { // white pawn
                        Square square(file, rank);
                        set_ref_bb(pawnCaptureStepsBB[White][origin], square);
                    }
                    else if (dir == SouthEast || dir == SouthWest) { // black pawn
                        Square square(file, rank);
                        set_ref_bb(pawnCaptureStepsBB[Black][origin], square);
                    }
                }
            }
        }

        // Kings steps array initialization
        for (Square origin = a1; origin <= h8; ++origin) {
            Bitboard steps = 0;
            for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir + 1)) {
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
            for (int ko = 0; ko < 8; ++ko) {
                int file = knightStepOffsets[ko][0] + origin.file();
                int rank = knightStepOffsets[ko][1] + origin.rank();
                if (file >= 0 && file < 8 && rank >= 0 && rank < 8)
                    set_ref_bb(steps, Square(file, rank));
            }
            knightStepsBB[origin] = steps;
        }

        // Directional Piece rays initialization
        for (Square origin = a1; origin < SQUARE_CNT; ++origin) {
            for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir + 1)) {
                Square target = origin;
                Bitboard steps = 0;
                while (1) {
                    int file = directionStepOffsets[dir][0] + target.file();
                    int rank = directionStepOffsets[dir][1] + target.rank();
                    if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
                        target = Square(file, rank);
                        set_ref_bb(steps, target);
                    }
                    else {
                        break;
                    }
                }
                directionStepsBB[origin][dir] = steps;
            }
        }

        // From Square to Square direction initialization
        for (Square from = a1; from < SQUARE_CNT; ++from)
            for (Square to = a1; to < SQUARE_CNT; ++to)
                fromToDirection[from][to] = NO_DIRECTION;

        for (Square from = a1; from < SQUARE_CNT; ++from) {
            for (Direction dir = DIRECTION_FIRST; dir < DIRECTION_CNT; dir = Direction(dir + 1)) {
                for (Square to = a1; to < SQUARE_CNT; ++to) {
                    Bitboard ray = directionStepsBB[from][dir];
                    if (is_set_bb(ray, to))
                        fromToDirection[from][to] = dir;
                }
            }
        }
    }
} const globalsInitializerMoves;

/*-- Explicit template instantiations to link against templates in other source files --*/ 
template void generate_pawn_moves<White>(Position& pos, MoveList& moveList);
template void generate_pawn_moves<Black>(Position& pos, MoveList& moveList);

template void generate_moves_under_check<White>(Position& pos, MoveList& moveList);
template void generate_moves_under_check<Black>(Position& pos, MoveList& moveList);

template void generate_knight_moves<White>(Position& pos, MoveList& moveList);
template void generate_knight_moves<Black>(Position& pos, MoveList& moveList);

template void generate_sliding_piece_moves<White, Bishop>(Position& pos, MoveList& moveList);
template void generate_sliding_piece_moves<White, Rook>(Position& pos, MoveList& moveList);
template void generate_sliding_piece_moves<White, Queen>(Position& pos, MoveList& moveList);
template void generate_sliding_piece_moves<Black, Bishop>(Position& pos, MoveList& moveList);
template void generate_sliding_piece_moves<Black, Rook>(Position& pos, MoveList& moveList);
template void generate_sliding_piece_moves<Black, Queen>(Position& pos, MoveList& moveList);

template void generate_king_moves<White>(Position& pos, MoveList& moveList);
template void generate_king_moves<Black>(Position& pos, MoveList& moveList);

template void generate_nk_moves_to_target_sq<White>(Position& pos, Square target, MoveList& moveList);
template void generate_nk_moves_to_target_sq<Black>(Position& pos, Square target, MoveList& moveList);

template void generate_moves_under_check<White>(Position& pos, MoveList& moveList);
template void generate_moves_under_check<Black>(Position& pos, MoveList& moveList);