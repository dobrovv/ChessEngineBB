#ifndef ENGINETYPES_H
#define ENGINETYPES_H

#include <cstdint>
#include <cassert>
#include <algorithm>
#include <vector>

//#define DISABLE_TT

/*-- Debugging on/off --*/
#define gDebug false
//#undef assert
//#define assert() {}

/*-- shift left and shift right macros --*/
#define SHL(value, count) ( (std::uint64_t)(value) << (count) )
#define SHR(value, count) ( (std::uint64_t)(value) >> (count) )

enum SquareEnum : std::uint8_t {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    ///////////////////////////////
    SQUARE_CNT
};

constexpr std::uint8_t FILE_CNT = 8;
constexpr std::uint8_t RANK_CNT = 8;

enum PieceColor : std::uint8_t {
    White, Black,
    /////////////
    COLOR_CNT
};

constexpr PieceColor operator!(PieceColor color) { return color == White ? Black : White; }

enum PieceType : std::uint8_t {
    Empty, Pawn, Knight,
    Bishop, Rook, Queen, King,
    //////////////////////////
    TYPE_CNT
};

constexpr PieceType operator++(PieceType pt) { return pt = (PieceType)(pt + 1); }

/*
The directional offsets are represented by the following compass
noWe       nort         noEa
      + 7 + 8 + 9
         \  |  /
west - 1 < -0 -> + 1    east
         /  |  \
      - 9 - 8 - 7
soWe       sout         soEa
*/

enum Direction : std::int8_t {
    NorthWest,          // offset +7
    North,          // offset +8
    NorthEast,          // offset +9
    East,           // offset +1
    SouthEast,          // offset -7
    South,          // offset -8
    SouthWest,          // offset -9
    West,           // offset -1
    //////////////
    DIRECTION_CNT, 
    NO_DIRECTION = DIRECTION_CNT,   // no direcion specified
    DIRECTION_FIRST = NorthWest,    // direction to start the iterations with
};

//TODO add bishopDir and rookDir function cheks for directions
//TODO add direction positive/negative for rayPieceSteps

constexpr bool isDirectionPositive(Direction dir) {
    assert(dir != NO_DIRECTION);
    return dir < 4;
}

constexpr bool isDirectionNegative(Direction dir) {
    assert(dir != NO_DIRECTION);
    return dir >= 4;
}

constexpr bool isRookDirection(Direction dir) {
    assert(dir != NO_DIRECTION);
    return dir % 2 != 0;    // (dir == North || dir == East || dir == South || dir == West);
}

constexpr bool isBishopDirection(Direction dir) {
    assert(dir != NO_DIRECTION);
    return dir % 2 == 0;    // (dir == NorthWest || dir == NorthEast || dir == SouthEast || dir == SouthWest);
}

constexpr bool isDirectionRook(Direction dir) {
    assert(dir != NO_DIRECTION);
    return dir % 2 != 0;    // (dir == North || dir == East || dir == South || dir == West);
}

constexpr bool isDirectionBishop(Direction dir) {
    assert(dir != NO_DIRECTION);
    return dir % 2 == 0;    // (dir == NorthWest || dir == NorthEast || dir == SouthEast || dir == SouthWest);
}

/*-- inverts the direction dir --*/
constexpr Direction invDir(Direction dir) {
    assert(dir != NO_DIRECTION);
    return static_cast<Direction>( (dir + 4) % DIRECTION_CNT );
}

class Square {

    std::uint8_t ofst;

public:

    Square() = default;

    inline constexpr Square(std::uint8_t value) : ofst(value) {}
    inline constexpr Square(SquareEnum value)   : ofst(value) {}

    constexpr Square(std::uint8_t file, std::uint8_t rank) : ofst(file+rank*8) {}

    constexpr std::uint8_t file() const { return ofst & 0x07; }
    constexpr std::uint8_t rank() const { return ofst >> 3; }

    constexpr Square nextFile() const { return Square(ofst + 1); }
    constexpr Square prevFile() const { return Square(ofst - 1); }
    constexpr Square nextRank() const { return Square(ofst + 8); }
    constexpr Square prevRank() const { return Square(ofst - 8); }

    /* Bottom-Left to Top-Right */
    constexpr Square nextDiagMain() const { return Square(ofst + 9); }
    constexpr Square prevDiagMain() const { return Square(ofst - 9); }

    /* Bottom-Right to Top-Left */
    constexpr Square nextDiagAnti() const { return Square(ofst + 7); }
    constexpr Square prevDiagAnti() const { return Square(ofst - 7); }

    constexpr bool sameFile(Square other) const { return file() == other.file(); }
    constexpr bool sameRank(Square other) const { return rank() == other.rank(); }

    constexpr Square flipVertically() const { return ofst ^ 56; }
    constexpr Square flip() const { return ofst ^ 56; }

    inline std::uint8_t chessboardDistance(Square other) const {
        return std::max( std::abs( other.file() - file() ), std::abs( other.rank() - rank() ) );
    }

    //constexpr operator SquareEnum() const { return SquareEnum(ofst); }
    inline constexpr operator std::uint8_t() const { return ofst; }

    inline Square& operator++() { ofst++; return *this; }
    constexpr bool operator==(SquareEnum other) { return ofst == other; }
    constexpr bool operator!=(SquareEnum other) { return ofst != other; }

}; // !class Square


class Piece {

    PieceColor m_color;
    PieceType  m_type;

public:

    Piece() = default;

    constexpr Piece(PieceColor color, PieceType type) : m_color(color), m_type(type) {}

    constexpr PieceColor color() const { return m_color; }
    constexpr PieceType type()   const { return m_type; }

    constexpr bool sameColor(const Piece other) const { return m_color == other.m_color; }
    constexpr bool sameType(const Piece other) const { return m_type == other.m_type; }

    inline void setColor(PieceColor color) { m_color = color; }
    inline void setType(PieceType type)    { m_type = type; }


    constexpr bool isEmpty()  const { return m_type == Empty; }
    constexpr bool isPawn()   const { return m_type == Pawn; }
    constexpr bool isKnight() const { return m_type == Knight; }
    constexpr bool isBishop() const { return m_type == Bishop; }
    constexpr bool isRook()   const { return m_type == Rook; }
    constexpr bool isQueen()  const { return m_type == Queen; }
    constexpr bool isKing()   const { return m_type == King; }

    constexpr bool isWhite() const { return m_color == White; }
    constexpr bool isBlack() const { return m_color == Black; }

}; // !class Piece

enum MoveType : std::uint8_t {
    // MoveType class defines possible move types
    // Reference: https://chessprogramming.wikispaces.com/Encoding+Moves
    //                          +-------------------------------------------+
    // Fits into 4 bit as flags |   bit 3   |  bit 2  |   bit 1  |   bit 0  |
    //                          | Promotion | Capture | Special1 | Special0 |

    Special_0 = SHL(1, 0),
    Special_1 = SHL(1, 1),
    ///////////////////////////
    QuietMove = 0,
    DoublePush = QuietMove | Special_0,
    CastleKSide = QuietMove | Special_1,
    CastleQSide = QuietMove | Special_0 | Special_1,
    ////////////////////////////////////////////////////
    Capture = SHL(1, 2),
    CaptureEnPas = Capture | Special_0,
    //////////////////////////////////////
    Promotion = SHL(1, 3),
    PromToKnight = Promotion,
    PromToBishop = Promotion | Special_0,
    PromToRook = Promotion | Special_1,
    PromToQueen = Promotion | Special_0 | Special_1,
    ///////////////////////////////////////////////////////
    PromToKnightCapture = PromToKnight | Capture,
    PromToBishopCapture = PromToBishop | Capture,
    PromToRookCapture = PromToRook | Capture,
    PromToQueenCapture = PromToQueen | Capture,
}; // !enum MoveType

class Move
{
    std::uint16_t bits;
    // | Move bits meaning:                              |
    // | ----------------------------------------------- |
    // | Promotion | Capture | Special | Target | Origin |
    // |     15    |    14   |  13-12  |  11-8  |  5-0   |
    // | ----------------------------------------------- |
    // |              Type             | Target | Origin |


public:

    constexpr Move()
        : bits(0) {}
    
    constexpr Move(uint16_t bits) 
        : bits(bits) {}

    constexpr Move(Square origin, Square target, MoveType type)
        : bits( (origin & 0x3F) | ((target & 0x3F) << 6) | ((type & 0x0F) << 12) ) {}

    constexpr Square   origin() const { return Square(bits & 0x3F); }
    constexpr Square   target() const { return Square((bits >> 6) & 0x3F); }
    constexpr MoveType type()   const { return MoveType((bits >> 12) & 0x0F); }

    constexpr bool isCapture()   const { return bits & (1<<14); }
    constexpr bool isCastle()    const { return type() & (QuietMove | Special_1); }
    constexpr bool isPromotion() const { return bits & (1<<15); }
    constexpr bool isValid()     const { return bits; }

    inline PieceType promoteTo() const {
        std::uint8_t toType = type() & ~(1<<2); // remove capture flag
        assert(isPromotion());
        return PieceType(Knight + (toType & ~(1 << 3))); // remove promotion flag
    }

    constexpr std::uint16_t as_bits() const { return bits; }

    constexpr bool operator==(Move other) const { return bits == other.bits; }
    constexpr bool operator!=(Move other) const { return bits != other.bits; }
}; // !class Move


#define NullMove Move(0)

/*-- A value respresenting the move ordering priority --*/
using Sort = int16_t;

/*-- Extended move --*/
struct SortMove : Move {
    Sort sort; 
    
    SortMove() {}

    SortMove(Move mv)
        : Move(mv) {};

    SortMove(Square origin, Square target, MoveType type)
        : Move(origin, target, type) {}
};

/*-- Evaluation value --*/
using Value = int32_t;

/*-- A value representing the engine's evaluation value in centi-pawns --*/
using Score = int32_t;

/*-- Time in ms --*/
using Time = uint64_t;

/*-- Represents a checkmate or the maximum / minimum achievable score --*/
constexpr Score CHECKMATE_SCORE = 100000;//INT_MAX; // note INT_MIN = -INT_MAX-1

/*-- Zobrist key --*/
using Key = uint64_t;

struct ExtMove {
    Move move;
    Score score;
};


#endif // ENGINETYPES_H
