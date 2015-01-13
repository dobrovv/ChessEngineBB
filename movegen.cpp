//#include "movegen.h"
//#include "bitboard.h"

//struct InitMoveGen {
//    InitMoveGen() {
//        // Knight moves array initialization
//        for (Square origin = a1; origin <= h8; ++origin) {
//            Bitboard moves = 0;
//            for (int ko=0; ko < 8; ++ko) {
//                int file = knightOffsets[ko][0] + origin.file();
//                int rank = knightOffsets[ko][1] + origin.rank();
//                if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
//                    set_ref_bb(moves, Square(file,rank));
//                }
//            }
//            MoveGen::knightMoves[origin] = moves;
//        }

//        // Piece direction rays initialization
//        for (Square origin = a1; origin < SQUARE_CNT; ++origin) {
//            for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
//                Square target = origin;
//                Bitboard moves = 0;
//                while(1) {
//                    int file = dirOffsets[dir][0] + target.file();
//                    int rank = dirOffsets[dir][1] + target.rank();
//                    if (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
//                        target = Square(file,rank);
//                        set_ref_bb(moves, target);
//                    } else  {
//                        break;
//                    }
//                }
//                MoveGen::directionMoves[origin][dir] = moves;
//            }
//        }

//        // Square direction initialization
//        for (Square from = a1; from < SQUARE_CNT; ++from)
//            for (Square to = a1; to < SQUARE_CNT; ++to)
//                MoveGen::squareDirection[from][to] = DIRECTION_CNT;

//        for (Square from = a1; from < SQUARE_CNT; ++from) {
//            for (Direction dir = North; dir < DIRECTION_CNT; dir = Direction(dir+1)) {
//                for (Square to = a1; to < SQUARE_CNT; ++to) {
//                    Bitboard ray = MoveGen::directionMoves[from][dir];
//                    if (is_set_bb(ray, to))
//                        MoveGen::squareDirection[from][to] = dir;
//                }
//            }
//        }

//    }
//} const initMoveGen;



//void MoveGen::movesForSide(const Board &brd, PieceColor side, std::vector<Move> &moveList)
//{
//    if (side == White) {
//        movesForPawns<White>(brd, moveList);
//        movesForKnights<White>(brd, moveList);
//        movesForRayPieces<White, Bishop>(brd, moveList);
//        movesForRayPieces<White, Rook>(brd, moveList);
//        movesForRayPieces<White, Queen>(brd, moveList);
//    } else {
//        movesForPawns<Black>(brd, moveList);
//        movesForKnights<Black>(brd, moveList);
//        movesForRayPieces<Black, Bishop>(brd, moveList);
//        movesForRayPieces<Black, Rook>(brd, moveList);
//        movesForRayPieces<Black, Queen>(brd, moveList);
//    }
//}
