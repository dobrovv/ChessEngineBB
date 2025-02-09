#include <vector>
#include <tuple>
#include <string>
#include <iostream>

#include "board.h"

using namespace std;

// If parameter is not true, test fails
// This check function would be provided by the test framework
#define IS_TRUE(x) { if (!(x)) std::cout << __FUNCTION__ << " failed on line " << __LINE__ << std::endl; }

uint64_t perft(Board& b, int depth);

bool test_movegen() {
    
    // Perft results wiki: https://www.chessprogramming.org/Perft_Results
	// Table of (fen, depth, nodes) for test positions
	vector<tuple<string, int, uint64_t>> perftTestPositions = {
		{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609 },  // Initial
        { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4, 4085603 }, // Kiwipete
        { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 6, 11030083},
        { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292},
        { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487},
        { "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594}
	};

    Board b;

    for (auto pos : perftTestPositions) {
        cout << "POSITION: [" << std::get<0>(pos) << "] DEPTH: " << std::get<1>(pos) << " NODES:  " << std::get<2>(pos) << " STATUS: ";
        
        b = Board::fromFEN(std::get<0>(pos));
        uint64_t nodes = perft(b, std::get<1>(pos));

        if (nodes == std::get<2>(pos))
            cout << " PASS" << endl;
        else {
            cout << "FAIL" << endl; 
            return false;
        }
    }

    return true;
}

bool do_tests() {
    bool pass = test_movegen();
    cout << "=======================" << endl;
    if (pass)
        cout << "Tests passed";
    else
        cout << "Tests failed";

    return pass;
}