#include <iostream>
#include <string>

#include <thread>

#include "board.h"
#include "tests.h"
#include "util.h"
#include "engine.h"
#include "search.h"

using namespace std;


// Reads a command line from the standart input and returns an array of tokens
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

// Reads UCI go commands from the standart input and returns a search request struct.
SearchRequest getSearchRequestArgs(vector<string>& args) {
    
    SearchRequest result;

    for ( int i = 0; i < args.size(); i++ ) {
        auto arg = args[i];
        
        if ( arg == "go" ) {

        } else if ( arg == "wtime" ) {
            result.time[White] = std::stoull(args[i + 1]);
            result.limits |= TimeLimit;
            i++;
        } else if ( arg == "btime" ) {
            result.time[Black] = std::stoull(args[i + 1]);
            result.limits |= TimeLimit;
            i++;
        } else if ( arg == "winc" ) {
            result.inc[White] = std::stoull(args[i + 1]);
            result.limits |= TimeLimit;
            i++;
        } else if ( arg == "binc" ) {
            result.inc[Black] = std::stoull(args[i + 1]);
            result.limits |= TimeLimit;
            i++;
        } else if ( arg == "movestogo" ) {
            result.movestogo = std::stoi(args[i + 1]);
            i++;
        } else if ( arg == "movetime" ) {
            result.movetime = std::stoull(args[i + 1]);
            result.limits |= TimeLimit;
            i++;
        } else if ( arg == "depth" ) {
            result.depth = std::stoi(args[i + 1]);
            result.limits |= DepthLimit;
            i++;
        } else if ( arg == "nodes" ) {
            result.nodes = std::stoull(args[i + 1]);
            result.limits |= NodesLimit;
            i++;
        } else if ( arg == "infinite" ) {
            result.limits |= InfiniteLimit;
        } else {
            cout << "info string error unknown go arg " << arg << endl;
        }
    }

    return result;
}

// apply moves in UCI notation to the board 'startPos'
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
            cout << "id name ChessZombie v0.14" << endl;
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
        else if ( args.size() >= 2 && args[0] == "d") {
            if ( args.size() >= 3 && args[1] == "piece" ) {
                int piece = std::stoi(args[2]);
                cout << "White" << endl;
                print_bb(board.pieces(White,(PieceType)piece));
                cout << "Black" << endl;
                print_bb(board.pieces(Black, (PieceType)piece));
            } else if ( args.size() >= 2 && args[1] == "colored" ) {
                cout << "White" << endl;
                print_bb(board.colored(White));
                cout << "Black" << endl;
                print_bb(board.colored(Black));
                cout << "Both" << endl;
                print_bb(board.occupied());
            }
        }
        else if ( args.size() >= 1 && args[0] == "go" ) {
            
            ExtMove result;
            start_search(board, result, getSearchRequestArgs(args));
        
        } else if (args.size() == 1 && (args[0] == "q" || args[0] == "quit" || args[0] == "exit")) {
            exit(0);
        } else if (args.size() == 1 && args[0] == "test") {
            do_tests();
        } else {
            //cout << "Invalid command." << endl;
        }
    }
    
    return 0;
}

