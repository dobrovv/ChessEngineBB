#include "util.h"

ostream& print_board(const Board& b, ostream& os) {
    os << "+------------------------+\n";
    for ( Square i = a1; i < SQUARE_CNT; ++i ) {
        if ( i.file() == 0 ) {
            os << "|                        |\n";
            os << "|";
        }
        Piece p = b.pieceAt(i.flipVertically());
        switch ( p.type() ) {
        case Empty:  os << " . "; break;
        case Pawn:   os << (p.color() == White ? " P " : " p "); break;
        case Knight: os << (p.color() == White ? " N " : " n "); break;
        case Bishop: os << (p.color() == White ? " B " : " b "); break;
        case Rook:   os << (p.color() == White ? " R " : " r "); break;
        case Queen:  os << (p.color() == White ? " Q " : " q "); break;
        case King:   os << (p.color() == White ? " K " : " k "); break;
        default:;
        }
        if ( i.file() == 7 ) {
            os << "|\n";
        }
    }
    os << "|                        |\n";
    os << "+------------------------+";
    return os;
}

/*-- prints board with row ranks and column files --*/ 
ostream& print_board_with_files(const Board& b, ostream& os) {
    os << "   a  b  c  d  e  f  g  h  \n";
    os << " +------------------------+\n";
    for ( Square i = a1; i < SQUARE_CNT; ++i ) {
        if ( i.file() == 0 ) {
            os << " |                        |\n";
            os << (int)i.flipVertically().rank() + 1 << "|";
        }
        Piece p = b.pieceAt(i.flipVertically());
        switch ( p.type() ) {
        case Empty:  os << " . "; break;
        case Pawn:   os << (p.color() == White ? " P " : " p "); break;
        case Knight: os << (p.color() == White ? " N " : " n "); break;
        case Bishop: os << (p.color() == White ? " B " : " b "); break;
        case Rook:   os << (p.color() == White ? " R " : " r "); break;
        case Queen:  os << (p.color() == White ? " Q " : " q "); break;
        case King:   os << (p.color() == White ? " K " : " k "); break;
        default:;
        }
        if ( i.file() == 7 ) {
            os << "|" << (int)i.flipVertically().rank() + 1 << "\n";
        }
    }
    os << " |                        |\n";
    os << " +------------------------+\n";
    os << "   a  b  c  d  e  f  g  h  ";

    os << "\n\nFEN: " << b.toFEN() << "\n";
    os << "Key: " << b.positionState().hash << (b.hashPosition(b) == b.positionState().hash ? "" : "\tINVALID") << "\n";

    return os;
}

ostream& print_bb(const Bitboard& bitboard, ostream& os ) {

    for ( Square i = a1; i <= h8; ++i ) {
        if ( i.file() == 0 )
            os << '\n';
        bool isSet = is_set_bb(bitboard, i.flipVertically());
        os << (isSet ? "1" : ".");
    }
    return os << std::endl;
}

ostream& print_square(const Square& square, ostream& os ) {
    char symbolic[3] = "";
    symbolic[0] = 'a' + square.file();
    symbolic[1] = '1' + square.rank();
    return os << symbolic;
}

//print move in UCI notation
ostream& print_move(const Move& move, ostream& os) {
    print_square(move.origin(), os);
    //os << (move.isCapture() ? "x": "-");
    print_square(move.target(), os);
    if ( move.isPromotion() ) {
        assert(move.promoteTo() != Empty);
        switch ( move.promoteTo() ) {
        case Queen: os << "q"; break;
        case Knight: os << "n"; break;
        case Bishop: os << "b"; break;
        case Rook: os << "r"; break;
        }
    }
    return os;
}

ostream& print_score(Score score, ostream& os) {
    if ( IsCheckMateScore(score) ) {
        cout << "mate " << value_mated_in(score);
    } else {
        cout << "cp " << score;
    }
    return os;
}

ostream& print_pv_moves(Board& b, ostream& os ) {
    std::vector<Move> pv = getPrimaryVariation(b);
    for ( auto mv : pv ) {
        print_move(mv, os);
        os << " ";
    }

    return os;
}

/*-- Verifies that the UCI notation move string can be played and returns a valid engine move --*/
Move verify_move(Board& b, std::string moveStr) {
    bool isOk = false;
    Move retMove{}; // invalid
    MoveList moveList;

    b.movesForSide(b.sideToMove(), moveList);


    for ( Move move : moveList ) {
        std::stringstream ss;
        print_move(move, ss);
        if ( moveStr == ss.str() ) {
            retMove = move;
            return retMove;
        }
    }

    return retMove;
}

uint64_t perft(Board& b, int depth) {

    uint64_t nodes_cnt = 0;

    MoveList moveList;

    if ( depth == 0 ) {
        return 1;
    } else if ( depth == 1 ) {
        generate_all_moves(b, moveList);
        return moveList.size();
    } else {

        generate_all_moves(b, moveList);

        for ( Move move : moveList ) {

            b.moveDo(move);
            nodes_cnt += perft(b, depth - 1);
            b.moveUndo();
        }
    }
    return nodes_cnt;

}

uint64_t divide(Board& b, int depth) {

    MoveList moveList;
    uint64_t sumNodes = 0;
    b.movesForSide(b.sideToMove(), moveList);
    for ( Move move : moveList ) {
        uint64_t nodes = 0;
        b.moveDo(move);
        nodes += perft(b, depth - 1);
        sumNodes += nodes;
        b.moveUndo();
        print_move(move) << ": " << nodes << "\n";
    }

    return sumNodes;
}

/* -- 
-- Get a primary variation (from TT)
-- TODO: currently there is no repetition detection, so a pv line can contain a loop repeating moves for a certain position
-- */
std::vector<Move> getPrimaryVariation(Board& b) {
    std::vector<Move> pvmvs;

    //b.moveDo(bestMove);
    //pvmvs.push_back(bestMove);

    int maxPv = 50; // limit max size of the pv line to avoid repetition

    // make the best known move - the first in pv sequence
    // get the next move from the TT table
    TTEntry* cell = b.ttable()->entry(b.pstate().hash);
    while ( cell->hashkey == b.pstate().hash && maxPv > 0 ) {
        MoveList legalMoves;
        generate_all_moves(b, legalMoves);

        // verify that the hashed move is a legal move
        bool isLegal = false;
        for ( auto legalMove : legalMoves )
            if ( cell->ttMove == legalMove )
                isLegal = true;

        if ( !isLegal )
            break;

        b.moveDo(cell->ttMove);
        pvmvs.push_back(cell->ttMove);
        cell = b.ttable()->entry(b.pstate().hash);
        maxPv--;
    }

    // undo moves
    for ( int i = pvmvs.size() - 1; i >= 0; --i ) {
        b.moveUndo();
    }

    return pvmvs;
}