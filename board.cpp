#include "board.h"
#include <sstream>
#include <iterator>

Board::Board()
{
}

void Board::moveDo(Move move)
{
    MoveType moveType = move.type();
    Piece piece = pieceAt(move.origin() );

    /* handle state changes */

    // save state of the position
    state_stack.push_back(positionState());

    // set en passant square
    if (moveType == DoublePush) {
        state.epSquare = piece.color() == White
                ? move.target().prevRank()
                : move.target().nextRank();
    } else {
        state.epSquare = NOT_ENPASSANT;
    }

    // handle half move clock for the fifty-move rule
    if (moveType == Capture || piece.isPawn()) {
        ++state.halfmove_clock;
    } else {
        state.halfmove_clock = 0;
    }

    // handle castling rights
    if (state.castle_rights) {
        if (move.origin() == e1 ) { // white king moves
            state.castle_rights &= ~(CastlingFlagWK & CastlingFlagWQ);
        } else if (move.origin() == e8) { // black king moves
            state.castle_rights &= ~(CastlingFlagBK & CastlingFlagBQ);
        } else if (move.origin() == a1) { // wq rook moves
            state.castle_rights &= ~(CastlingFlagWQ);
        } else if (move.origin() == h1) { // wk rook moves
            state.castle_rights &= ~(CastlingFlagWK);
        } else if (move.origin() == a8) { // bq rook moves
            state.castle_rights &= ~(CastlingFlagBQ);
        } else if (move.origin() == h8) { // bk rook moves
            state.castle_rights &= ~(CastlingFlagBK);
        }
    }

    /* make positional changes */
    if (moveType == QuietMove) {
        removePiece(move.origin() );
        setPiece(piece, move.target() );

    } else if (moveType == Capture) {
        captured_pieses.push_back(pieceAt(move.target() )); // store captured piece
        removePiece(move.origin() );
        removePiece(move.target() );
        setPiece(piece, move.target() );

    } else if (moveType == DoublePush) {
        removePiece(move.origin() );
        setPiece(piece, move.target() );

    } else if (move.isPromotion() ) {
        if (move.isCapture() ) {
            captured_pieses.push_back(pieceAt(move.target() )); // store captured piece
            removePiece(move.target() );
        }
        removePiece(move.origin() );
        piece.setType(move.promoteTo() );
        setPiece(piece, move.target() );

    } else if (move.isCastle()) {
        if (move.type() == CastleQSide) {
            const Square rookQSide   = piece.color() == White ? Square(a1) : Square(a8);
            const Square rookQTarget = piece.color() == White ? Square(d1) : Square(d8);
            setPiece(pieceAt(rookQSide), rookQTarget);
            removePiece(rookQSide);
            removePiece(move.origin() );
            setPiece(piece, move.target() );
        } else {
            const Square rookKSide   = piece.color() == White ? Square(h1) : Square(h8);
            const Square rookKTarget = piece.color() == White ? Square(f1) : Square(f8);
            setPiece(pieceAt(rookKSide), rookKTarget);
            removePiece(rookKSide);
            removePiece(move.origin() );
            setPiece(piece, move.target() );
        }

    } else if (moveType == CaptureEnPas) {
        Square capturedPawn = piece.color() == White
                ? move.target().prevRank()
                : move.target().nextRank();
        captured_pieses.push_back(pieceAt(capturedPawn) ); // store captured pawn
        removePiece(capturedPawn);
        removePiece(move.origin() );
        setPiece(piece, move.target() );
    }

    moves_done.push_back(move);
    side = !side;
}

void Board::moveUndo()
{

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

    return board;
}
