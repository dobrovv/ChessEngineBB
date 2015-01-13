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
    Piece p = piece_at[square];

    reset_ref_bb(occupied_bb, square);
    reset_ref_bb(colored_bb[p.color()], square);
    reset_ref_bb(pieces_bb[p.color()][p.type()], square);

    piece_at[square] = Piece(White, Empty);
}

void Position::movesForSide(PieceColor color, std::vector<Move> &moveList)
{
    /* if (color == White) {
        for (Square square = 0; square < 64; ++square) {
            if (!is_set_bb(colored(White), square) )
                movesToSquare<White>(square, moveList);
        }
    } else {
        for (Square square = 0; square < 64; ++square) {
            if (!is_set_bb(colored(Black), square))
                movesToSquare<Black>(square, moveList);
        }
    }*/

    if (color == White && isKingAttacked<White>() ) {
        movesForKing<White>(moveList);
        return;
    } else if (color == Black && isKingAttacked<Black>() ) {
        movesForKing<Black>(moveList);
        return;
    }

    if (color == White) {
        movesForPawns<White>(moveList);
        movesForKnights<White>(moveList);
        movesForRayPieces<White, Bishop>(moveList);
        movesForRayPieces<White, Rook>(moveList);
        movesForRayPieces<White, Queen>(moveList);
        movesForKing<White>(moveList);
    } else {
        movesForPawns<Black>(moveList);
        movesForKnights<Black>(moveList);
        movesForRayPieces<Black, Bishop>(moveList);
        movesForRayPieces<Black, Rook>(moveList);
        movesForRayPieces<Black, Queen>(moveList);
        movesForKing<Black>(moveList);
    }
}

template<PieceColor color>
void Position::movesForPawns(std::vector<Move> &moveList)
{
    Square target;
    Square origin;
    Bitboard promoted;

    Bitboard pushed = pawnsPush<color>();
    promoted = color == White
            ? pushed & Rank8_bb
            : pushed & Rank1_bb;
    pushed ^= promoted;

    while (promoted) {
        target = pop_lsb(promoted);
        origin = color == White
                ? target.prevRank()
                : target.nextRank();
        moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
        moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
    }
    while (pushed) {
        target = pop_lsb(pushed);
        origin = color == White
                ? target.prevRank()
                : target.nextRank();
        moveList.emplace_back(origin, target, QuietMove);
    }

    Bitboard doublePushed = pawnsPushDouble<color>();

    while (doublePushed) {
        target = pop_lsb(doublePushed);
        origin = color == White
                ? target.prevRank().prevRank()
                : target.nextRank().nextRank();
        moveList.emplace_back(origin, target, DoublePush);
    }

    Bitboard eastCapture = pawnsCaptureEast<color>();
    promoted = color == White
            ? eastCapture & Rank8_bb
            : eastCapture & Rank1_bb;
    eastCapture ^= promoted;

    while (promoted) {
        target = pop_lsb(promoted);
        origin = color == White
                ? target.prevDiagMain()
                : target.nextDiagAnti();
        moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
        moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
    }

    while (eastCapture) {
        target = pop_lsb(eastCapture);
        origin = color == White
                ? target.prevDiagMain()
                : target.nextDiagAnti();
        moveList.emplace_back(origin, target, Capture);
    }

    Bitboard westCapture = pawnsCaptureWest<color>();
    promoted = color == White
            ? westCapture & Rank8_bb
            : westCapture & Rank1_bb;
    westCapture ^= promoted;

    while (promoted) {
        target = pop_lsb(promoted);
        origin = color == White
                ? target.prevDiagAnti()
                : target.nextDiagMain();
        moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
        moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
    }

    while (westCapture) {
        target = pop_lsb(westCapture);
        origin = color == White
                ? target.prevDiagAnti()
                : target.nextDiagMain();
        moveList.emplace_back(origin, target, Capture);
    }

    // En passant handling
    if (state.epSquare != NOT_ENPASSANT) {
        Bitboard pawns_set = pawnCaptureStepsBB[!color][state.epSquare] & pieces(color, Pawn);
        while (pawns_set) {
            Square origin = pop_lsb(pawns_set);
            moveList.emplace_back(origin, state.epSquare, CaptureEnPas);
        }
    }

}

template<PieceColor color>
void Position::movesForKnights(std::vector<Move>& moveList)
{
    Bitboard knights = (color == White)
            ? pieces(White, Knight)
            : pieces(Black, Knight);

    while (knights) {
        Square origin = pop_lsb(knights);
        Bitboard quietMoves = knightStepsBB[origin] & ~occupied();

        while (quietMoves) {
            Square target = pop_lsb(quietMoves);
            moveList.emplace_back(origin, target, QuietMove);
        }

        Bitboard captureMoves = (color==White)
                ? colored(Black) & knightStepsBB[origin]
                  : colored(White) & knightStepsBB[origin];

        while (captureMoves) {
            Square target = pop_lsb(captureMoves);
            moveList.emplace_back(origin, target, Capture);
        }
    }
}

template<PieceColor color, PieceType pieceType>
void Position::movesForRayPieces(std::vector<Move>& moveList) {

    assert(pieceType == Queen || pieceType == Rook || pieceType == Bishop);

    Bitboard pieces_set = (color == White)
            ? pieces(White, pieceType)
            : pieces(Black, pieceType);

    while (pieces_set) {
        Square origin = pop_lsb(pieces_set);
        Direction dir = (pieceType == Bishop)
                ? NorthEast         // Start with diagolnal direction eg bishop
                : North;            // Start with orthogonal direction eg queen and rook

        while (dir < DIRECTION_CNT) {
            Bitboard steps = rayPieceSteps(occupied(), origin, dir);
            Bitboard capture = steps & occupied();
            if (capture) {
                Square captureTarget =  bitscan_forward(capture);
                // forward/reverse doesn't matter as there should be only one capture
                //if (pieceAt(captureTarget).color() != color)
                if (capture & colored(!color))
                    moveList.emplace_back(origin, captureTarget, Capture);
                reset_ref_bb(steps, captureTarget); // delete captureTarget
            }
            while(steps) {
                Square target = pop_lsb(steps);
                moveList.emplace_back(origin, target, QuietMove);
            }
            // Rotate direction
            dir = (pieceType == Queen)
                    ? Direction(dir+1)  // by 45° for queen
                    : Direction(dir+2); // by 90° for rook and bishop
        }
    }
}

template<PieceColor color>
void Position::movesForKing(std::vector<Move> &moveList)
{
    Bitboard king_set = pieces(color, King);
    Square origin;
    if (king_set) {

        origin = bitscan_forward(king_set);
        Bitboard steps = kingStepsBB[origin];
        steps ^= steps & colored(color);    // delete same colored pieces subset
        Bitboard  capture = steps & colored(!color);
        steps ^= capture;   // delete captures subset

        while (capture) {
            Square target = pop_lsb(capture);
            if (!isAttacked<!color>(target) )
                moveList.emplace_back(origin, target, Capture);
        }

        while (steps) {
            Square target = pop_lsb(steps);
            if (!isAttacked<!color>(target) )
                moveList.emplace_back(origin, target, QuietMove);
        }

        // castling
        const Square RookQSide   = color == White ? Square(a1) : Square(a8);
        const Square targetQSide = color == White ? Square(c1) : Square(c8);

        bool canQCastle = color == White
                ? state.castle_rights & CastlingFlagWQ
                : state.castle_rights & CastlingFlagBQ;

        const Square RookKSide   = color == White ? Square(h1) : Square(h8);
        const Square targetKSide = color == White ? Square(g1) : Square(g8);

        bool canKCastle = color == White
                ? state.castle_rights & CastlingFlagWK
                : state.castle_rights & CastlingFlagBK;

        if (canQCastle) {
            Bitboard rookQSteps = rayPieceSteps(occupied(), RookQSide, East);
            if (popcount_bb(rookQSteps) == 4) {
                pop_lsb(rookQSteps);   // unused - not on the kings path
                while (rookQSteps) {
                    Square sq = pop_lsb(rookQSteps);
                    if (isAttacked<!color>(sq) ) {
                        canQCastle = false;
                        break;
                    }
                }
                if (canQCastle) {
                    moveList.emplace_back(origin, targetQSide, CastleQSide);
                }
            }
        }

        if (canKCastle) {
            Bitboard rookKSteps = rayPieceSteps(occupied(), RookKSide, West);
            if (popcount_bb(rookKSteps) == 3) {
                while (rookKSteps) {
                    Square sq = pop_lsb(rookKSteps);
                    if (isAttacked<!color>(sq)) {
                        canKCastle = false;
                        break;
                    }
                }
                if (canKCastle) {
                    moveList.emplace_back(origin, targetKSide, CastleKSide);
                }
            }
        }
    }
}

// TODO: add castling as a way to reach the target
template<PieceColor color>
void Position::movesToSquare(Square target, std::vector<Move>& moveList) {
    Bitboard pieces_set = attackers<color>(target);

    if (is_set_bb(colored(color), target)) {
        pieces_set &= ~colored(color); // remove same colored pieces
    }
    if (!is_set_bb(colored(!color), target)) { // target is not an enemy piece

        pieces_set &= ~(pieces_set & pieces(color, Pawn)); // remove pawn captures

        //add pawn that can move to target square
        if (is_set_bb(pawnsPush<color>(), target)) {
            Square origin = color == White ? target.prevRank() : target.nextRank();
            //moveList.emplace_back(origin, target, QuietMove);
            set_ref_bb(pieces_set, origin);
        }

        //add double pushed pawn that can move to target square
        if (is_set_bb(pawnsPushDouble<color>(), target)) {
            Square origin = color == White ? target.prevRank().prevRank() : target.nextRank().nextRank();
            //moveList.emplace_back(origin, target, DoublePush);
            set_ref_bb(pieces_set, origin);
        }
    }

    while (pieces_set) {
        Square origin = pop_lsb(pieces_set);
        //moveList.emplace_back(origin, target, /*here should be the magic*/)
        Piece pieceOrig = pieceAt(origin);
        Piece pieceTrgt = pieceAt(target);

        if (pieceOrig.isPawn() ) { // is a pawn
            if (target == state.epSquare) {
                moveList.emplace_back(origin, target, CaptureEnPas);
            } else if ((color == White && target.rank() == 7) || (color == Black && target.rank() == 0)) {
                if ( pieceTrgt.isEmpty() ) {    // promote without capture
                    moveList.emplace_back(origin, target, PromToQueen); moveList.emplace_back(origin, target, PromToKnight);
                    moveList.emplace_back(origin, target, PromToRook); moveList.emplace_back(origin, target, PromToBishop);
                } else {    // promote with capture
                    moveList.emplace_back(origin, target, PromToQueenCapture); moveList.emplace_back(origin, target, PromToKnightCapture);
                    moveList.emplace_back(origin, target, PromToRookCapture); moveList.emplace_back(origin, target, PromToBishopCapture);
                }
            } else {
                moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture );
            }
        } else { // knight, bishop, rook, king, queen
            moveList.emplace_back(origin, target, pieceTrgt.isEmpty() ? QuietMove : Capture );
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
        return SHL(pieces(White,Pawn), 8) & ~occupied();
    } else {
        return SHR(pieces(Black,Pawn), 8) & ~occupied();
    }
}

template<PieceColor color>
Bitboard Position::pawnsPushDouble() {
    if (color == White) {
        Bitboard firstPush = SHL(pieces(White,Pawn) & Rank2_bb, 8) & ~occupied();
        return SHL(firstPush, 8) & ~occupied();
    } else {
        Bitboard firstPush = SHR(pieces(Black,Pawn) & Rank7_bb, 8) & ~occupied();
        return SHR(firstPush, 8) & ~occupied();
    }
}

template<PieceColor color>
Bitboard Position::pawnsCaptureEast() {
    if (color == White) {
        return SHL((pieces(White,Pawn) & ~FileH_bb), 9) & colored(Black);
    } else {
        return SHR((pieces(Black,Pawn) & ~FileH_bb), 7) & colored(White);
    }
}

template<PieceColor color>
Bitboard Position::pawnsCaptureWest() {
    if (color == White) {
        return SHL((pieces(White,Pawn) & ~FileA_bb), 7) & colored(Black);
    } else {
        return SHR((pieces(Black,Pawn) & ~FileA_bb), 9) & colored(White);
    }
}
