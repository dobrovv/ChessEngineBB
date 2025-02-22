#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"
#include "position.h"
#include <vector>
#include <string> // for FEN
#include <memory>
#include <utility> // max

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

    void moveDoNull();
    void moveUndoNull();

    bool detectRepetition() const;

    inline TT* ttable() { return table.get(); }

    std::vector<Move> getPrimaryVariation();

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

    friend int main();
};

/* -- 
-- Detect draws by the threefold repetition rule
-- Iterates trought the made moves and compares zobrist key to the position
-- */
inline bool Board::detectRepetition() const {
    int stateSize = state_stack.size();
    int movesToSearch = std::min((int)pstate().halfmove_clock, stateSize);
    
    const Key phash = pstate().hash;
    int repetitions = 0;

    /* TODO: (?)verify, i can be incremented by 2 */
    for ( int i = 1; i <= movesToSearch; i+=1 ) {
        if ( state_stack[stateSize-i].hash == phash ) {
            repetitions++;
            if ( repetitions >= 2 ) return true;
        }
    }
    return false;
}

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
