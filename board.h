#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"
#include "position.h"
#include <vector>
#include <string> // for FEN
#include <memory>

#include "movegen.h"
#include "evaluate.h"
#include "tt.h"

class Board : public Position
{
public:
//private:
    std::vector<Move>  moves_done;
    std::vector<PositionState> state_stack;
    std::vector<Piece> captured_pieses;
    std::shared_ptr<TT> table;

public:
    Board();
    ~Board();
    void moveDo(Move move);
    void moveUndo();


    std::vector<Move> getPrimaryVariation(Move bestMove);

    static Board fromFEN(std::string fenRecord);
    std::string toFEN() const;
    
    static Board startpos();

    // Generate zobrist key for position pos
    static Key hashPosition(const Position& pos);

    template <PieceColor Color>
    uint32_t getMobilityScore();

    template <PieceColor Color>
    inline uint32_t getMobilityForKnight(Square origin);

    template <PieceColor Color>
    inline uint32_t getMobilityForBishop(Square origin);

    template <PieceColor Color>
    inline uint32_t getMobilityForRook(Square origin);

    template <PieceColor Color>
    inline uint32_t getMobilityForQueen(Square origin);

    // Evaluates the current position and returns a value in centipawns
    Value evaluate();
    
    // Search for the best move
    uint64_t search(ExtMove& result, int depth = 5);




};

template <PieceColor Color>
uint32_t Board::getMobilityScore() {
    
    constexpr PieceColor Ally = Color == White ? White : Black;
    Position& pos = *this;
    
    find_pinned_pieces<Ally>(pos);
    return mobility_score_for<Ally>(pos);
}

template <PieceColor Color>
inline uint32_t Board::getMobilityForKnight(Square origin) {
    
    constexpr PieceColor Ally = Color == White ? White : Black;
    Position& pos = *this;

    find_pinned_pieces<Ally>(pos);
    return mobility_for_knight<Ally>(pos, origin);
}

template <PieceColor Color>
uint32_t Board::getMobilityForBishop(Square origin) {
    constexpr PieceColor Ally = Color == White ? White : Black;
    Position& pos = *this;

    find_pinned_pieces<Ally>(pos);
    return mobility_for_bishop<Ally>(pos, origin);
}

template <PieceColor Color>
uint32_t Board::getMobilityForRook(Square origin) {
    constexpr PieceColor Ally = Color == White ? White : Black;
    Position& pos = *this;
    
    find_pinned_pieces<Ally>(pos);
    return mobility_for_rook<Ally>(pos, origin);
}

template <PieceColor Color>
uint32_t Board::getMobilityForQueen(Square origin) {
    constexpr PieceColor Ally = Color == White ? White : Black;
    Position& pos = *this;

    find_pinned_pieces<Ally>(pos);
    return mobility_for_queen<Ally>(pos, origin);
}



#endif // BOARD_H
