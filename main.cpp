#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "board.h"

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

uint64_t perft(Board& b, int depth){
    if (depth == 0)
        return 1;

    std::vector<Move> moveList;
    moveList.reserve(256);
    uint64_t nodes = 0;
    b.movesForSide(b.sideToMove(), moveList);

    for (Move move : moveList) {

        b.moveDo(move);
        //print_board(b);
        //cout << "\n";
        //cin.get();
        nodes += perft(b, depth - 1);
        b.moveUndo();
    }
    return nodes;

}

int main()
{

    Board board = Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t result = perft(board, 5);
    auto stop_time = std::chrono::high_resolution_clock::now();

    int delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
    delta_ms = std::max(delta_ms, 1); // prevent the good old divide by 0 problem :)

    cout << "Perft Nodes: " <<  result << " " << delta_ms << " ms " << (result / delta_ms * 1000) << " nodes/s" << endl;
    return 0;

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

        possibleMoves.clear();
        board.movesForSide(board.sideToMove(), possibleMoves);
    }

    return 0;
}

