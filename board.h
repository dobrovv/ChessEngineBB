#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"
#include "position.h"
#include <vector>
#include <string> // for FEN

class Board : public Position
{
public:
//private:
    std::vector<Move>  moves_done;
    std::vector<PositionState> state_stack;
    std::vector<Piece> captured_pieses;


public:
    Board();
    void moveDo(Move move);
    void moveUndo();

    static Board fromFEN(std::string fenRecord);
    
    static Board startpos();

};
#endif // BOARD_H
