//#ifndef MOVEGEN_H
//#define MOVEGEN_H

//#include "types.h"
//#include "board.h"

//namespace MoveGen
//{
//    extern Bitboard knightMoves[SQUARE_CNT];
//    extern Bitboard directionMoves[SQUARE_CNT][DIRECTION_CNT];

//    extern Direction squareDirection[SQUARE_CNT][SQUARE_CNT];
//    //                                 from          to

//    template<PieceColor color> Bitboard pawnsPush(const Board& brd);
//    template<PieceColor color> Bitboard pawnsPushDouble(const Board& brd);
//    template<PieceColor color> Bitboard pawnsCaptureEast(const Board& brd);
//    template<PieceColor color> Bitboard pawnsCaptureWest(const Board& brd);

//    inline Bitboard pieceRayAttacks(Bitboard occupied, Square origin, Direction dir);

//    void movesForSide(const Board& board, PieceColor color, std::vector<Move>& moveList);

//    template<PieceColor color>
//    void movesForPawns(const Board& brd, std::vector<Move>& moveList)
//    {
//        Square target;
//        Square origin;

//        if (color == White) {
//            Bitboard pushed = pawnsPush<White>(brd);
//            while (pushed) {
//                target = pop_lsb(pushed);
//                origin = target.prevRank();
//                moveList.emplace_back(origin, target, QuietMove);
//            }

//            Bitboard doublePushed = pawnsPushDouble<White>(brd);
//            while (doublePushed) {
//                target = pop_lsb(doublePushed);
//                origin = target.prevRank().prevRank();
//                moveList.emplace_back(origin, target, DoublePush);
//            }

//            Bitboard eastCapture = pawnsCaptureEast<White>(brd);
//            while (eastCapture) {
//                target = pop_lsb(eastCapture);
//                origin = target.prevDiagMain();
//                moveList.emplace_back(origin, target, Capture);
//            }

//            Bitboard westCapture = pawnsCaptureWest<White>(brd);
//            while (westCapture) {
//                target = pop_lsb(westCapture);
//                origin = target.prevDiagAnti();
//                moveList.emplace_back(origin, target, Capture);
//            }

//        } else {    // side == black
//            Bitboard pushed = pawnsPush<Black>(brd);
//            while (pushed) {
//                target = pop_lsb(pushed);
//                origin = target.nextRank();
//                moveList.emplace_back(origin, target, QuietMove);
//            }

//            Bitboard doublePushed = pawnsPushDouble<Black>(brd);
//            while (doublePushed) {
//                target = pop_lsb(doublePushed);
//                origin = target.nextRank().nextRank();
//                moveList.emplace_back(origin, target, DoublePush);
//            }

//            Bitboard eastCapture = pawnsCaptureEast<Black>(brd);
//            while (eastCapture) {
//                target = pop_lsb(eastCapture);
//                origin = target.nextDiagAnti();
//                moveList.emplace_back(origin, target, Capture);
//            }

//            Bitboard westCapture = pawnsCaptureWest<Black>(brd);
//            while (westCapture) {
//                target = pop_lsb(westCapture);
//                origin = target.nextDiagMain();
//                moveList.emplace_back(origin, target, Capture);
//            }
//        }
//    }

//    template<PieceColor color>
//    void movesForKnights(const Board &brd, std::vector<Move>& moveList)
//    {
//        Bitboard knights = (color == White)
//                ? brd.pieces(White, Knight)
//                : brd.pieces(Black, Knight);

//        while (knights) {
//            Square origin = pop_lsb(knights);
//            Bitboard quietMoves = knightMoves[origin] & ~brd.occupied();

//            while (quietMoves) {
//                Square target = pop_lsb(quietMoves);
//                moveList.emplace_back(origin, target, QuietMove);
//            }

//            Bitboard captureMoves = (color==White)
//                    ? brd.colored(Black) & knightMoves[origin]
//                    : brd.colored(White) & knightMoves[origin];

//            while (captureMoves) {
//                Square target = pop_lsb(captureMoves);
//                moveList.emplace_back(origin, target, Capture);
//            }
//        }
//    }

//    template<PieceColor color, PieceType pieceType>
//    void movesForRayPieces(const Board &brd, std::vector<Move>& moveList) {

//        assert(pieceType == Queen || pieceType == Rook || pieceType == Bishop);

//        Bitboard pieces = (color == White)
//                ? brd.pieces(White, pieceType)
//                : brd.pieces(Black, pieceType);

//        while (pieces) {
//            Square origin = pop_lsb(pieces);
//            Direction dir = (pieceType == Bishop)
//                    ? NorthEast         // Start with diagolnal direction eg bishop
//                    : North;            // Start with orthogonal direction eg queen and rook

//            while (dir < DIRECTION_CNT) {
//                Bitboard moves = pieceRayAttacks(brd.occupied(), origin, dir);
//                Bitboard capture = moves & brd.occupied();
//                if (capture) {
//                    Square captureTarget =  bitscan_forward(capture);
//                        // forward/reverse doesn't matter as there should be only one capture
//                    if (brd.pieceAt(captureTarget).color() != color)
//                        moveList.emplace_back(origin, captureTarget, Capture);
//                    reset_ref_bb(moves, captureTarget); // delete captureTarget
//                }
//                while(moves) {
//                    Square target = pop_lsb(moves);
//                    moveList.emplace_back(origin, target, QuietMove);
//                }
//                // Rotate direction
//                dir = (pieceType == Queen)
//                        ? Direction(dir+1)  // by 45° for queen
//                        : Direction(dir+2); // by 90° for rook and bishop
//            }
//        }
//    }

//    template<PieceColor color>
//    Bitboard pawnsPush(const Board& brd) {
//        if (color == White) {
//            return SHL(brd.pieces(White,Pawn), 8) & ~brd.occupied();
//        } else {
//            return SHR(brd.pieces(Black,Pawn), 8) & ~brd.occupied();
//        }
//    }

//    template<PieceColor color>
//    Bitboard pawnsPushDouble(const Board& brd) {
//        if (color == White) {
//            Bitboard firstPush = SHL(brd.pieces(White,Pawn) & Rank2_bb, 8) & ~brd.occupied();
//            return SHL(firstPush, 8) & ~brd.occupied();
//        } else {
//            Bitboard firstPush = SHR(brd.pieces(Black,Pawn) & Rank7_bb, 8) & ~brd.occupied();
//            return SHR(firstPush, 8) & ~brd.occupied();
//        }
//    }

//    template<PieceColor color>
//    Bitboard pawnsCaptureEast(const Board& brd) {
//        if (color == White) {
//            return SHL((brd.pieces(White,Pawn) & ~FileH_bb), 9) & brd.colored(Black);
//        } else {
//            return SHR((brd.pieces(Black,Pawn) & ~FileH_bb), 7) & brd.colored(White);
//        }
//    }

//    template<PieceColor color>
//    Bitboard pawnsCaptureWest(const Board& brd) {
//        if (color == White) {
//            return SHL((brd.pieces(White,Pawn) & ~FileA_bb), 7) & brd.colored(Black);
//        } else {
//            return SHR((brd.pieces(Black,Pawn) & ~FileA_bb), 9) & brd.colored(White);
//        }
//    }

//    // Sliding Piece Attacks source:
//    // https://chessprogramming.wikispaces.com/Classical+Approach
//    inline Bitboard pieceRayAttacks(Bitboard occupied, Square origin, Direction dir) {
//        Bitboard ray = directionMoves[origin][dir];
//        Bitboard blocker = ray & occupied;
//        if (blocker) {
//            Square blockerSquare = (dir < 3 || dir == 7)
//                    ? bitscan_forward(blocker)      // direction is positive - scan forward
//                    : bitscan_reverse(blocker);     // direction is nevatige - scan reverse
//            ray ^= directionMoves[blockerSquare][dir];
//        }
//        return ray;
//    }
//}

//#endif // MOVEGEN_H
