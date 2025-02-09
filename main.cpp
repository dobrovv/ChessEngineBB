#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

#include "board.h"
#include "tests.h"

using namespace std;

ostream& print_board(const Board& b, ostream& os = std::cout) { 
    os << "+------------------------+\n";
    for (Square i = a1; i < SQUARE_CNT; ++i) {
        if ( i.file() == 0) {
            os << "|                        |\n";
            os << "|";
        }
        Piece p = b.pieceAt(i.flipVertically());
        switch (p.type()) {
            case Empty:  os << " . "; break;
            case Pawn:   os << (p.color() == White ? " P " : " p "); break;
            case Knight: os << (p.color() == White ? " N " : " n "); break;
            case Bishop: os << (p.color() == White ? " B " : " b "); break;
            case Rook:   os << (p.color() == White ? " R " : " r "); break;
            case Queen:  os << (p.color() == White ? " Q " : " q "); break;
            case King:   os << (p.color() == White ? " K " : " k "); break;
            default:;
        }
        if ( i.file() == 7) {
            os << "|\n";
        }
    }
    os << "|                        |\n";
    os << "+------------------------+";
    return os;
}

// prints board with row ranks and column files
ostream& print_board_with_files(const Board& b, ostream& os = std::cout) {
    os << "   a  b  c  d  e  f  g  h  \n";
    os << " +------------------------+\n";
    for (Square i = a1; i < SQUARE_CNT; ++i) {
        if (i.file() == 0) {
            os << " |                        |\n";
            os << (int)i.flipVertically().rank() + 1 << "|";
        }
        Piece p = b.pieceAt(i.flipVertically());
        switch (p.type()) {
        case Empty:  os << " . "; break;
        case Pawn:   os << (p.color() == White ? " P " : " p "); break;
        case Knight: os << (p.color() == White ? " N " : " n "); break;
        case Bishop: os << (p.color() == White ? " B " : " b "); break;
        case Rook:   os << (p.color() == White ? " R " : " r "); break;
        case Queen:  os << (p.color() == White ? " Q " : " q "); break;
        case King:   os << (p.color() == White ? " K " : " k "); break;
        default:;
        }
        if (i.file() == 7) {
            os <<  "|" << (int)i.flipVertically().rank() + 1  << "\n" ;
        }
    }
    os << " |                        |\n";
    os << " +------------------------+\n";
    os << "   a  b  c  d  e  f  g  h  ";

    os << "\n\nFEN: " << b.toFEN() << "\n";
    os << "Key: " << b.positionState().hash << (b.hashPosition(b) == b.positionState().hash ? "" : "\tINVALID") << "\n";

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

//print move in UCI notation
ostream& print_move(const Move& move, ostream& os = std::cout) {
    print_square(move.origin(), os);
    //os << (move.isCapture() ? "x": "-");
    print_square(move.target(), os);
    if (move.isPromotion()) {
        assert(move.promoteTo() != Empty);
        switch (move.promoteTo()) {
            case Queen: os << "q"; break;
            case Knight: os << "n"; break;
            case Bishop: os << "b"; break;
            case Rook: os << "r"; break;
        }
    }
    return os;
}

ostream& print_pv_moves(Board& b, Move firstSeqMove, ostream& os = std::cout) {
    vector<Move> pv = b.getPrimaryVariation(firstSeqMove);
    for ( auto mv : pv ) {
        print_move(mv, os);
        os << " ";
    }
    
    return os;
}

/*Verifies that the specified move in moveStr as UCI notation string can be played and returns a valid engine move */
Move verify_move(Board& b, std::string moveStr) {
    bool isOk = false;
    Move retMove {}; // invalid
    MoveList moveList;
    
    b.movesForSide(b.sideToMove(), moveList);

    
    for (Move move : moveList) {
        std::stringstream ss;
        print_move(move, ss);
        if (moveStr == ss.str()) {
            retMove = move;
            return retMove;
        }
    }

    return retMove;
}

Move randomMove(Board & b) {
    MoveList moveList;
    b.movesForSide(b.sideToMove(), moveList);
    if (moveList.size() > 0)
        return moveList[rand() % moveList.size()];
    else
        return Move();
}


uint64_t perft(Board& b, int depth){
    
    uint64_t nodes_cnt = 0;

    MoveList moveList;

    if (depth <= 1) {
        generate_all_moves(b, moveList);
        return moveList.size();
    } else {

        generate_all_moves(b, moveList);

        for (Move move : moveList) {

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
    for (Move move : moveList) {
        uint64_t nodes = 0;
        b.moveDo(move);
        nodes += perft(b, depth - 1);
        sumNodes += nodes;
        b.moveUndo();
        print_move(move) << ": " << nodes << "\n";
    }

    return sumNodes;
}

// reads a command line from the standart input and returns each token
vector<string> getCommandArgs(void) {
    string cmdArgs;
    
    std::getline(std::cin, cmdArgs);
    istringstream iss(cmdArgs);

    vector<string> tokens;
    copy(istream_iterator<string>(iss),
        istream_iterator<string>(),
        back_inserter(tokens));

    return tokens;
}

// apply moves in an UCI notation to the board 'startPos'
Board getBoardFromMoves(vector<string> uciMoves, Board startPos = Board::startpos()) {
    
    for(string move : uciMoves ) {
        Move nextMove = verify_move(startPos, move);
        if (nextMove.isValid()) {
            startPos.moveDo(nextMove);
        }
        else {
            //cout << "Invalid move. " << move << endl;
            return Board::startpos();
        }
    }

    return startPos;
}



int main()
{
    string uciMove;
    Move currentMove;

    Board board = Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");    
    
    /*Initial
        position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
        perft(6) = 119,060,324
        perft(7) = 3,195,901,860
    */
    /*Kiwipete:
        position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - moves
        peft(6) = 8,031,647,685
     */
    /*Position 3
        position fen 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -
        perft(8) = 3,009,794,393
    */
    /*Other:
        https://www.chessprogramming.org/Lasker-Reichhelm_Position
    */
    //Board board = Board::fromFEN("8/8/3p4/KPp4r/1R2Pp1k/8/6P1/8 b - e3");
    
    //cout.imbue(std::locale("")); // use commas to decorate large integers

    while ( 1 ) {
        vector<string> args = getCommandArgs();
        if ( args.size() == 1 && args[0] == "uci" ) {
            cout << "id name Chekov v0.13" << endl;
            cout << "id author dobrovv" << endl;
            cout << "uciok" << endl;
        } else if (args.size() == 1 && args[0] == "isready" ) {
            cout << "readyok" << endl;
        } else if ( args.size() == 2 && args[0] == "position" && args[1] == "startpos" ) {
            board = getBoardFromMoves(vector<string>());
        }
        else if (args.size() >= 3 && args[0] == "position" && args[1] == "startpos" && args[2] == "moves") {
            vector<string> moves = std::vector<string>(args.begin() + 3, args.end());
            board = getBoardFromMoves(moves);
            //print_board_with_files(board) << "\n";
        }
        else if (args.size() >= 2 && args[0] == "position" && args[1] == "fen") {
            std::stringstream fenSS;
            vector<string> moves;
            
            // read fen arguments
            int i;
            
            for (i = 2; i < args.size(); i++) {
                if (args[i] != "moves") {
                    fenSS << args[i] << " ";
                }
                else break;
            }
            
            if (i < args.size() && args[i] == "moves") {
                moves = std::vector<string>(args.begin() + i + 1, args.end());
            }
            
            Board startPos = Board::fromFEN(fenSS.str());
            board = getBoardFromMoves(moves, startPos);
            //print_board_with_files(board) << "\n";

        }
        else if (args.size() == 1 && args[0] == "d") {
            print_board_with_files(board) << "\n";
            
            //cout << "isKingAttacked:" << board.isKingAttacked<White>() << " " << board.isKingAttacked<Black>() << endl;
            //cout << "isAbsolutlyPinned:" << board.isAbsolutelyPinned<White>(d2) << endl;
            //cout << "isEnPasCaptureLegal:" << board.isEnPasCaptureLegal<White>(c5) << endl; 
            //cout << "E.p file and rank: " << (char)('a' + board.state.epSquare.file()) << " " << (int)(board.state.epSquare.rank() + 1) << endl;
            //cout << "Pinned pieces" << endl; print_bb(board.pinned_bb);
        }
        else if (args.size() == 1 && args[0] == "eval") {
            cout << "Evaluation: " << board.evaluate() << "cp" << endl;
            
            Bitboard allyKnights = board.pieces(White, Knight);
            Bitboard allyBishops = board.pieces(White, Bishop);
            Bitboard allyRooks = board.pieces(White, Rook);
            Bitboard allyQueens = board.pieces(White, Queen);
            
            cout << "-----------------------" << endl;
            foreach_pop_lsb(origin, allyKnights) {
                cout << "Knight at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForKnight<White>(origin) << endl;
            }

            foreach_pop_lsb(origin, allyBishops) {
                cout << "Bishop at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForBishop<White>(origin) << endl;
            }

            foreach_pop_lsb(origin, allyRooks) {
                cout << "Rook at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForRook<White>(origin) << endl;
            }

            foreach_pop_lsb(origin, allyQueens) {
                cout << "Queen at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForQueen<White>(origin) << endl;
            }
            
            cout << "-----------------------" << endl;

            allyKnights = board.pieces(Black, Knight);
            allyBishops = board.pieces(Black, Bishop);
            allyRooks = board.pieces(Black, Rook);
            allyQueens = board.pieces(Black, Queen);

            foreach_pop_lsb(origin, allyKnights) {
                cout << "Knight at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForKnight<Black>(origin) << endl;
            }

            foreach_pop_lsb(origin, allyBishops) {
                cout << "Bishop at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForBishop<Black>(origin) << endl;
            }

            foreach_pop_lsb(origin, allyRooks) {
                cout << "Rook at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForRook<Black>(origin) << endl;
            }

            foreach_pop_lsb(origin, allyQueens) {
                cout << "Queen at ";
                print_square(origin);
                cout << " mobility " << board.getMobilityForQueen<Black>(origin) << endl;
            }

            cout << "-----------------------" << endl;

        }
        else if ((args.size() == 3 && args[0] == "go" && args[1] == "perft")) {
            int depth = std::stoi(args[2]);
            auto start_time = std::chrono::high_resolution_clock::now();
            uint64_t result = divide(board, depth);
            auto stop_time = std::chrono::high_resolution_clock::now();

            int delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
            delta_ms = std::max(delta_ms, 1); // prevent the good old divide by 0 problem :)

            cout << "Perft(" << depth << "): " << result << " nodes " << delta_ms << "ms " << (result / delta_ms * 1000) << " nodes/s" << endl;
            cout << " " << endl;
        }
        else if (args.size() == 3 && args[0] == "go" && args[1] == "depth") {
            ExtMove result;
            int depth = std::stoi(args[2]);

            auto start_time = std::chrono::high_resolution_clock::now();
            uint64_t nodeCount = board.search(result, depth);
            auto stop_time = std::chrono::high_resolution_clock::now();
            
            auto delta_ms = std::max(std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count(), 1LL);

            
            /*cout << "    value: " << result.score << " c.p.   " << nodeCount << " nodes " << delta_ms << "ms " << (nodeCount / delta_ms * 1000) << " nodes/s" << endl;
            cout << "Primary variation: ";
            print_pv_moves(board, result.move);
            cout << endl;*/

            Value relativeScore = result.score * (board.sideToMove() ? -1 : 1);

            cout << "info " << "depth " << depth << " score cp " << relativeScore << " time " << delta_ms << " nodes " << nodeCount << " nps " << (nodeCount / delta_ms * 1000) << " pv "; print_pv_moves(board, result.move); cout << endl;
            cout << "bestmove "; print_move(result.move); cout << endl; 

        }
        /*else if (args.size() == 3 && args[0] == "go" && args[1] == "depthAB") {
            ExtMove result;
            int depth = std::stoi(args[2]);

            auto start_time = std::chrono::high_resolution_clock::now();
            uint64_t nodesSearched = board.searchAB(result, depth);
            auto stop_time = std::chrono::high_resolution_clock::now();

            int delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count();
            delta_ms = std::max(delta_ms, 1);

            cout << "Best move: ";
            print_move(result.move);
            cout << "    value: " << result.score << " c.p.   " << nodesSearched << " nodes " << delta_ms << "ms " << (nodesSearched / delta_ms * 1000) << " nodes/s" << endl;
        }*/
        else if (args[0] == "go" ) {
            
            const int depth = 5;
            ExtMove result;
            
            auto start_time = std::chrono::high_resolution_clock::now();
            uint64_t nodeCount = board.search(result, depth);
            auto stop_time = std::chrono::high_resolution_clock::now();

            auto delta_ms = std::max(std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time).count(), 1LL);

            Value relativeScore = result.score * (board.sideToMove() ? -1 : 1);

            cout << "info " << "depth " << depth << " score cp " << relativeScore << " time " << delta_ms << " nodes " << nodeCount << " nps " << (nodeCount / delta_ms * 1000) << " pv "; print_pv_moves(board, result.move); cout << endl;
            
            cout << "bestmove "; print_move(result.move); cout << endl;
        
        } else if (args.size() == 1 && (args[0] == "q" || args[0] == "quit" || args[0] == "exit")) {
            exit(0);
        } else if (args.size() == 1 && args[0] == "test") {
            do_tests();
        } else {
            //cout << "Invalid command." << endl;
        }

        
        /*
        while (!currentMove.isValid()) {
            cin >> uciMove;
            currentMove = verify_move(board, uciMove);
            if (!currentMove.isValid()) {
                cout << "Invalid move." << endl;
            }
        }

        
        depth--;

        board.moveDo(currentMove);
       */
    }
    
    return 0;
}

