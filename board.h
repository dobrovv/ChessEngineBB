#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"
#include "position.h"
#include <vector>

class Board : public Position
{
    std::vector<Move>  moves_done;
    std::vector<PositionState> state_stack;
    std::vector<Piece> captured_pieses;


public:
    Board();
    void moveDo(Move move);
    void moveUndo();

    static Board fromFEN(std::string fenRecord);

};
#endif // BOARD_H
