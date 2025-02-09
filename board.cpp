#include "board.h"
#include "search.h"
#include <iterator>
#include <iostream>
#include <sstream> // for FEN 
#include <limits> // INT_MAX, INT_MIN
#include <algorithm> //std::min, std::max

Board::Board()
{
    table = std::make_shared<TT>(256);
}

Board::~Board() {
    //free(table);
}

std::vector<Move> Board::getPrimaryVariation(Move bestMove) {
    Board& b = *this;
    std::vector<Move> pvmvs;

    b.moveDo(bestMove);
    pvmvs.push_back(bestMove);
    
    // make the best known move - the first in pv sequence
    // get the next move from the TT table
    TTEntry* cell = b.table->cell(b.state.hash);
    while (cell->key == b.state.hash) {
        MoveList legalMoves;
        generate_all_moves(b, legalMoves);

        // verify that the hashed move is legal
        bool isLegal = false;
        for ( Move legalMove : legalMoves )
            if ( cell->move == legalMove )
                isLegal = true;

        if ( !isLegal )
            break;

        b.moveDo(cell->move);
        pvmvs.push_back(cell->move);
        cell = b.table->cell(b.state.hash);
    }

    // undo moves
    for (int i = pvmvs.size() - 1; i >= 0; --i) {
        b.moveUndo();
    }

    return pvmvs;
}

void Board::moveDo(Move move)
{

    const MoveType moveType = move.type();
    const Square origin     = move.origin();
    const Square target     = move.target();
    Piece piece = pieceAt(origin);
    PieceType pieceType = piece.type();

    /* handle state changes */

    // save the previous state of the position
    state_stack.emplace_back(positionState());

    // Remove en passant square from the hash
    // Note that state here referes still to the old state before the move
    if (state.epSquare != NOT_ENPASSANT)
        state.hash ^= Zobrist::enpassant[state.epSquare.file()];

    // set en passant square
    if (moveType == DoublePush) {
        state.epSquare = piece.color() == White
                ? target.prevRank()
                : target.nextRank();

        // Hash the enpassant move
        state.hash ^= Zobrist::enpassant[state.epSquare.file()];

    } else {
        state.epSquare = NOT_ENPASSANT;
    }

    // handle half move clock for the fifty-move rule
    if (moveType == Capture || piece.isPawn()) {
        state.halfmove_clock = 0;
    } else {
        ++state.halfmove_clock;
    }

    // handle castling rights
    const Bitboard originBB   = SHL(1,origin);
    constexpr Bitboard castleOriginSquares = SHL(1,e1) | SHL(1,e8)
            | SHL(1,a1) | SHL(1,h1) | SHL(1,a8) | SHL(1,h8);

    if (castleOriginSquares & originBB) {
        if (origin == e1 ) { // white king moves
            state.castle_rights &= ~(CastlingFlagWK | CastlingFlagWQ);
        } else if (origin == e8) { // black king moves
            state.castle_rights &= ~(CastlingFlagBK | CastlingFlagBQ);
        } else if (origin == a1) { // wq rook moves
            state.castle_rights &= ~(CastlingFlagWQ);
        } else if (origin == h1) { // wk rook moves
            state.castle_rights &= ~(CastlingFlagWK);
        } else if (origin == a8) { // bq rook moves
            state.castle_rights &= ~(CastlingFlagBQ);
        } else if (origin == h8) { // bk rook moves
            state.castle_rights &= ~(CastlingFlagBK);
        }

        // Hash the change in castling rights
        state.hash ^= Zobrist::castling[state_stack.back().castle_rights]; // remove old castling flags
        state.hash ^= Zobrist::castling[state.castle_rights];              // set new castling flags
    }

    /* make positional changes */
    if (moveType == QuietMove) {
        removePiece(origin );
        setPiece(piece, target);

    } else if (moveType == Capture) {
        captured_pieses.emplace_back(pieceAt(target)); // store captured piece
        removePiece(origin);
        removePiece(target);
        setPiece(piece, target);

    } else if (moveType == DoublePush) {
        removePiece(origin);
        setPiece(piece, target);

    } else if (move.isPromotion() ) {
        if (move.isCapture() ) {
            captured_pieses.emplace_back(pieceAt(target)); // store captured piece
            removePiece(target);
        }
        removePiece(origin);
        piece.setType(move.promoteTo() );
        setPiece(piece, target);

    } else if (move.isCastle()) {
        if (move.type() == CastleQSide) {
            const Square rookQSide   = piece.color() == White ? Square(a1) : Square(a8);
            const Square rookQTarget = piece.color() == White ? Square(d1) : Square(d8);
            setPiece(pieceAt(rookQSide), rookQTarget);
            removePiece(rookQSide);
            removePiece(origin );
            setPiece(piece, target );
        } else {
            const Square rookKSide   = piece.color() == White ? Square(h1) : Square(h8);
            const Square rookKTarget = piece.color() == White ? Square(f1) : Square(f8);
            setPiece(pieceAt(rookKSide), rookKTarget);
            removePiece(rookKSide);
            removePiece(origin );
            setPiece(piece, target );
        }

    } else if (moveType == CaptureEnPas) {
        Square capturedPawn = piece.color() == White
                ? target.prevRank()
                : target.nextRank();
        captured_pieses.emplace_back(pieceAt(capturedPawn) ); // store captured pawn
        removePiece(capturedPawn);
        removePiece(origin );
        setPiece(piece, target );
    }
    else {
        assert(false); // invalid move in moveDo
    }

    moves_done.emplace_back(move);
    side = !side;
    state.hash ^= Zobrist::black_to_move;
}

void Board::moveUndo()
{
    const Move move = moves_done.back();
    
    const Square origin = move.origin();
    const Square target = move.target();
    
    const Piece pieceOrig = pieceAt(target);
    const MoveType moveType = move.type();


    if (moveType == QuietMove) {
        removePiece(target );
        setPiece(pieceOrig, origin );
    } else if (moveType == Capture) {
        removePiece(target );
        setPiece(captured_pieses.back(), target );
        captured_pieses.pop_back();
        setPiece(pieceOrig, origin );

    } else if (moveType == DoublePush) {
        removePiece(target );
        setPiece(pieceOrig, origin );
    } else if (move.isPromotion() ) {
        removePiece(target);
        if (move.isCapture() ) {
            setPiece(captured_pieses.back(), target );
            captured_pieses.pop_back();
        }
        setPiece(pieceOrig.color(), Pawn, origin);
    } else if (move.isCastle()) {
        if (move.type() == CastleQSide) {
            const Square rookQSide   = pieceOrig.color() == White ? Square(a1) : Square(a8);
            const Square rookQTarget = pieceOrig.color() == White ? Square(d1) : Square(d8);
            removePiece(rookQTarget);
            setPiece(pieceOrig.color(), Rook, rookQSide);
            removePiece(target );
            setPiece(pieceOrig, origin );
        } else {
            const Square rookKSide   = pieceOrig.color() == White ? Square(h1) : Square(h8);
            const Square rookKTarget = pieceOrig.color() == White ? Square(f1) : Square(f8);
            removePiece(rookKTarget);
            setPiece(pieceOrig.color(), Rook, rookKSide);
            removePiece(target );
            setPiece(pieceOrig, origin );
        }
    } else if (moveType == CaptureEnPas) {
        Square capturedPawn = pieceOrig.color() == White
                ? target.prevRank()
                : target.nextRank();
        setPiece(captured_pieses.back(), capturedPawn);
        captured_pieses.pop_back();
        removePiece(target );
        setPiece(pieceOrig, origin );
    } else {
        assert(false); // invalid move in moveUndo
    }

    moves_done.pop_back();
    side = !side;

    // restore state
    setPositionState(state_stack.back());
    state_stack.pop_back();

}

Key Board::hashPosition(const Position &pos) {

    Key key = 0;

    for (int sq = 0; sq < SQUARE_CNT; sq++) {
        Piece piece = pos.pieceAt(sq);
        if (!piece.isEmpty())
            key ^= Zobrist::pieces[piece.color()][sq][piece.type()];
    }

    if (pos.sideToMove() == Black)
        key ^= Zobrist::black_to_move;
    
    key ^= Zobrist::castling[pos.castling()];

    if ( pos.enPassantSq() != NOT_ENPASSANT )
        key ^= Zobrist::enpassant[pos.enPassantSq().file()];

    return key;
}

Board Board::fromFEN(std::string fenRecord)
{
    using namespace std;
    Board board;

    /* split fenRecord */
    istringstream iss(fenRecord);
    vector<string> records;
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(records));

    int rank = 7;
    int file = 0;
    /* find positions of the pieces */
    for (char& ch : records[0]) {
        if (ch == '/') {
            rank -= 1;
            file = 0;
        } else  if (std::isdigit(ch)) {
            int digit = ch - '0';
            file += digit;
        } else {
            Piece piece;

            char name = std::toupper(ch);
            if (name == 'P') {
                piece = Piece(std::isupper(ch) ? White : Black, Pawn);
            } else if (name == 'N') {
                piece = Piece(std::isupper(ch) ? White : Black, Knight);
            } else if (name == 'B') {
                piece = Piece(std::isupper(ch) ? White : Black, Bishop);
            } else if (name == 'R') {
                piece = Piece(std::isupper(ch) ? White : Black, Rook);
            } else if (name == 'Q') {
                piece = Piece(std::isupper(ch) ? White : Black, Queen);
            } else if (name == 'K') {
                piece = Piece(std::isupper(ch) ? White : Black, King);
            }

            board.setPiece(piece, Square(file, rank));
            ++file;
        }
    }

    /* get active color */
    if (records.size() > 1)
        board.setSideToMove(records[1] == "b" ? Black : White);

    /* get castling rights */
    if (records.size() > 2) {
        std::uint8_t castling = NoCastlingFlags;
        for (char& ch: records[2]) {
            if (ch == 'K')
                castling |= CastlingFlagWK;
            else if (ch == 'Q')
                castling |= CastlingFlagWQ;
            else if (ch == 'k')
                castling |= CastlingFlagBK;
            else if (ch == 'q')
                castling |= CastlingFlagBQ;
        }
        board.state.castle_rights = CastlingRights(castling);
    }

    /* get en passant square */
    if (records.size() > 3) {
        Square enPassant = NOT_ENPASSANT;
        if (records[3].size() == 2) {
            int file = records[3][0] - 'a';
            int rank = records[3][1] - '1';
            enPassant = Square(file, rank);
        }
        board.state.epSquare = enPassant;
    }

    /* get halfmove clock */
    if (records.size() > 4) {
        board.state.halfmove_clock = atoi(records[4].c_str());
    }

    board.state.hash = hashPosition(board);

    return board;
}

std::string Board::toFEN() const {
    
    const Position& pos = *this;

    int rank = 7;
    //int file = 0;

    std::string res;

    while (rank >= 0) {
        int fileOnRank = 0;
        int empty = 0;

        // Print pieces
        while (fileOnRank < 8) {
            Piece piece = pos.pieceAt(Square(fileOnRank, rank));

            // Count only empty squares and print the total number
            if (piece.isEmpty() && fileOnRank == 7) {
               res += std::to_string(empty+1);
               empty = 0;
            } else if ( piece.isEmpty() ) {
                empty++;
            } else if ( !piece.isEmpty() && empty ) {
                res += std::to_string(empty);
                empty = 0;
            }

            if ( piece.isPawn() ) {
                res += piece.isWhite() ? "P" : "p";
            } else if ( piece.isKnight() ) {
                res += piece.isWhite() ? "N" : "n";
            } else if ( piece.isBishop() ) {
                res += piece.isWhite() ? "B" : "b";
            } else if ( piece.isRook() ) {
                res += piece.isWhite() ? "R" : "r";
            } else if ( piece.isQueen() ) {
                res += piece.isWhite() ? "Q" : "q";
            } else if (piece.isKing()) {
                res += piece.isWhite() ? "K" : "k";
            }

            // Print / at the end of a rank unless rank is 0
            if (fileOnRank == 7 && rank != 0) {
                res += "/";
            }

            fileOnRank++;
        }
        
        rank--;
        fileOnRank = 0;
    }

    res += " ";

    // Print the side to move
    if (pos.sideToMove() == White)
        res += "w";
    else
        res += "b";

    res += " ";

    // Print castling rights
    std::uint8_t castling = pos.castling();
    if (castling == NoCastlingFlags) {
        res += "-";
    } else {
        if (castling & CastlingFlagWK)
            res += "K";
        if (castling & CastlingFlagWQ)
            res += "Q";
        if (castling & CastlingFlagBK)
            res += "k";
        if (castling & CastlingFlagBQ)
            res += "q";
    }


    res += " ";

    // Print en passant square
    Square enPassant = pos.enPassantSq(); 
    if ( enPassant != NOT_ENPASSANT ) {
        char symbolic[3] = "";
        symbolic[0] = 'a' + enPassant.file();
        symbolic[1] = '1' + enPassant.rank();
        res += symbolic;
    } else {
        res += "-";
    }

    res += " ";
    
    // Print halfmove_clock
    res += std::to_string(pos.halfmove_clock());

    res += " ";
    
    // Print move count
    res += "0";


    return res;
}


Board Board::startpos() {
    return Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Value Board::evaluate() {
    Position& pos = *this; 
    return ::evaluate(pos);
}

uint64_t Board::search(ExtMove& result, int depth) {
    return ::search(*this, result, depth);
}

// Positions taking too long to evaluate
// position fen rnbqkbnr/pppp1ppp/4p3/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 0
// position startpos moves e2e4 d7d5 f1d3 d5e4 d3e4 g8f6 e4f3 e7e5 b1c3 a7a5 d2d3 b8c6 d1e2 d8e7 g1h3 c6d4 e2d1 h7h5 e1g1 c8g4 c3d5 - takes too long on depth 5
// position startpos moves g1f3 d7d5 d2d4 c8f5 e2e3 d8d6 b1c3 b8c6 f1b5 a7a6 b5a4 b7b5 a4b3 -  takes longer on depth 4

// Position doesn't generate a best move using uci and go command
// r1b1k2r/1pQ2ppp/4p3/p6n/PbNP4/6P1/1PqN1KB1/R1B5 b kq - 2 21