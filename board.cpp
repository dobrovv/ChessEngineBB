#include "board.h"
#include <iterator>
#include <iostream>
#include <sstream> // for FEN 

Board::Board()
{
}

void Board::moveDo(Move move)
{

    const MoveType moveType = move.type();
    const Square origin     = move.origin();
    const Square target     = move.target();
    Piece piece = pieceAt(origin);

    /* handle state changes */

    // save the previous state of the position
    state_stack.emplace_back(positionState());

    // set en passant square
    if (moveType == DoublePush) {
        state.epSquare = piece.color() == White
                ? target.prevRank()
                : target.nextRank();
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
    static const Bitboard castleOriginSquares = SHL(1,e1) | SHL(1,e8)
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
    }

    /* make positional changes */
    if (moveType == QuietMove) {
        removePiece(origin );
        setPiece(piece, target );


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

Board Board::startpos() {
    return Board::fromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

int32_t Board::evaluate() {

    // weight in centipawns
    const static int PieceWeight[TYPE_CNT] = {
        0,      // Empty
        100,     // Pawn
        300,     // Knight,
        300,     // Bishop, 
        500,     // Rook, 
        900,     // Queen
        0,      // King
    };

    const static int MobilityWeight = 1;

    int32_t materialScore = 0;

    for ( PieceType pt = Pawn; pt < King; pt=PieceType(pt+1) ) {
        materialScore += PieceWeight[pt] * (popcount_bb(pieces(White, pt)) - popcount_bb(pieces(Black, pt)));
    }
    
    /*
    std::vector<Move> tmpWhiteMoves;
    std::vector<Move> tmpBlackMoves;
    tmpWhiteMoves.reserve(256);
    tmpBlackMoves.reserve(256);

    movesForSide(White, tmpWhiteMoves);
    movesForSide(Black, tmpBlackMoves);

    uint32_t whiteMobility = tmpWhiteMoves.size();
    uint32_t blackMobility = tmpBlackMoves.size();
    
    uint32_t mobilityScore = MobilityWeight * (tmpWhiteMoves.size() - tmpBlackMoves.size());
    */
    
    getPinnedPieces<White>();
    int32_t whiteMobility = getMobilityScore<White>();
    
    getPinnedPieces<Black>();
    int32_t blackMobility = getMobilityScore<Black>();

    int32_t mobilityScore = MobilityWeight * (whiteMobility - blackMobility);
    

    return materialScore+mobilityScore;
}



uint64_t Board::search(ExtMove& result, int depth) {
    int bestValue;
    Move bestMove;
    
    uint64_t nodesSearched
        = searchDo(depth, bestValue, bestMove);
    
    result.val = bestValue;
    result.move = bestMove;

    return nodesSearched;


}

uint64_t Board::searchDo(int depth, int& bestValue, Move& bestMove) {

    const PieceColor Side = sideToMove();

    int tmpValue = 1000000 * (Side == Black ? 1 : -1);
    Move tmpMove;
    uint64_t nodesSearched = 0;

    
    if (depth == 1) {
        std::vector<Move> moves;
        moves.reserve(256);
        movesForSide(sideToMove(), moves);

        for (Move move: moves) {
            moveDo(move);

            int val = evaluate();
            if (Side == White && val > tmpValue || Side == Black && val < tmpValue ) {
                tmpValue = val;
                tmpMove = move;
            }

            moveUndo();
        }
        nodesSearched += moves.size();

        bestValue = tmpValue;
        bestMove = tmpMove;
    }
    else {
        std::vector<Move> moves;
        moves.reserve(256);
        movesForSide(sideToMove(), moves);
        
        int val;
        Move moveBellow;

        for (Move move : moves) {
            moveDo(move);
            
            nodesSearched 
                += searchDo(depth - 1, val, moveBellow);
            
            if (Side == White && val > tmpValue || Side == Black && val < tmpValue) {
                tmpValue = val;
                tmpMove = move;
            }

            moveUndo();
        }

        bestValue = tmpValue;
        bestMove = tmpMove;
    }

    return nodesSearched;
    
}