#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "board.h"
#include "movegen.h"

using namespace std;

ostream& print_board(const Board& b, ostream& os = std::cout) {
    for (Square i = a1; i <= h8; ++i) {
        if ( i.file() == 0 )
            os << '\n';
        Piece p = b.pieceAt(i.flipVertically());
        switch (p.type()) {
        case Empty:  os << '.'; break;
        case Pawn:   os << (p.color() == White ? "P" : "p"); break;
        case Knight: os << (p.color() == White ? "N" : "n"); break;
        case Bishop: os << (p.color() == White ? "B" : "b"); break;
        case Rook:   os << (p.color() == White ? "R" : "r"); break;
        case Queen:  os << (p.color() == White ? "Q" : "q"); break;
        case King:   os << (p.color() == White ? "K" : "k"); break;
        default:;
        }
    }
    return os;
}

ostream& print_bb(const Bitboard& bitboard, ostream& os = std::cout) {

    for (Square i = a1; i <= h8; ++i) {
        if ( i.file() == 0 )
            os << '\n';
        bool isSet = is_set_bb(bitboard, i.flipVertically());
        os << (isSet ? "1": ".");
    }
    return os << endl;
}

ostream& print_square(const Square& square, ostream& os = std::cout) {
    char symbolic[3] = "";
    symbolic[0] = 'a' + square.file();
    symbolic[1] = '1' + square.rank();
    return os << symbolic;

}

ostream& print_move(const Move& move, ostream& os = std::cout) {
    print_square(move.origin());
    os << (move.isCapture() ? "x": "-");
    print_square(move.target());
    if (move.type() == CaptureEnPas){
        os << " e.p.";
    }
    return os;
}


int main()
{
//    Board b = Board::fromFEN("8/8/8/8/8/8/kpn5/P7 w");
//    print_board(b);
//    cout << endl;
//    print_bb(b.attackers<Black>(a1));
//    cout << endl;
//    print_bb(b.attackers<White>(b2));

//    return 0;

    Board board = Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");
    //Board board = Board::fromFEN("8/8/8/pppppppp/PPPPPPPP/8/8/8 w KQkq - 0 1");
    print_board(board);
    cout << endl;

    std::vector<Move> possibleMoves;
    board.movesForSide(board.sideToMove(), possibleMoves);

    int movecnt = 0;

    while (possibleMoves.size()) {
        cout << endl;
        Move randMove = possibleMoves[rand() % possibleMoves.size()];
        print_move(randMove) <<" #" << ++movecnt << endl;
        board.moveDo(randMove);
        print_board(board);
        cout << endl;
        if (randMove.isCapture())
            cin.get();

        //this_thread::sleep_for(chrono::milliseconds(1000));
        //cin.get();

        possibleMoves.clear();
        board.movesForSide(board.sideToMove(), possibleMoves);
    }

    return 0;
}

