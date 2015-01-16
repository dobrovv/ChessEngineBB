#ifndef ENGINETYPES_H
#define ENGINETYPES_H

#include <cstdint>
#include <cassert>
#include <algorithm>


// shift left and shift right macros
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

enum Direction {
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest,
    //////////////
    DIRECTION_CNT
};

constexpr std::int8_t directionStepOffsets[DIRECTION_CNT][2] = {{+0,+1}, {+1,+1}, {+1,0}, {+1,-1}, {+0,-1}, {-1,-1}, {-1,+0}, {-1,+1}};
constexpr std::int8_t knightStepOffsets[8][2] = { {+1,+2}, {+2,+1}, {+2,-1}, {+1,-2}, {-1,-2}, {-2,-1}, {-2,+1}, {-1,+2} };

enum MoveType : std::uint8_t {
    // MoveType implementation is taken from the chessprogramming site:
    // url: https://chessprogramming.wikispaces.com/Encoding+Moves
    // fits into 4 bits flags as | Promotion | Capture | Special1 | Special2 |

    Special_0       = SHL(1,0),
    Special_1       = SHL(1,1),
    ///////////////////////////
    QuietMove       = 0,
    DoublePush      = QuietMove | Special_0,
    CastleKSide     = QuietMove | Special_1,
    CastleQSide     = QuietMove | Special_0 | Special_1,
    ////////////////////////////////////////////////////
    Capture         = SHL(1,2),
    CaptureEnPas    = Capture | Special_0,
    //////////////////////////////////////
    Promotion       = SHL(1,3),
    PromToKnight    = Promotion,
    PromToBishop    = Promotion | Special_0,
    PromToRook      = Promotion | Special_1,
    PromToQueen     = Promotion | Special_0 | Special_1,
    ///////////////////////////////////////////////////////
    PromToKnightCapture = PromToKnight | Capture,
    PromToBishopCapture = PromToBishop | Capture,
    PromToRookCapture   = PromToRook   | Capture,
    PromToQueenCapture  = PromToQueen  | Capture,
}; // !enum MoveType

class Square {

    std::uint8_t ofst;

public:

    Square() = default;

    constexpr Square(std::uint8_t value) : ofst(value) {}
    constexpr Square(SquareEnum value)   : ofst(value) {}

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

    inline std::uint8_t chebyshevDistance(Square other) const {
        return std::max( std::abs( other.file() - file() ), std::abs( other.rank() - rank() ) );
    }

    constexpr operator int() const { return ofst; }
    constexpr operator std::uint8_t() const { return ofst; }
    constexpr operator SquareEnum() const { return SquareEnum(ofst); }

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

class Move
{
    std::uint16_t bits;
    // | Move bits meaning:                              |
    // | ----------------------------------------------- |
    // | Origin | Target | Special | Capture | Promotion |
    // |   0-5  |  6-11  |  12-13  |    14   |    15     |
    // | ----------------------------------------------- |


public:

    constexpr Move()
        : bits(0) {}

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
        return toType == PromToQueen ? Queen
                : toType == PromToKnight ? Knight
                  : toType == PromToRook ? Rook
                    : toType == PromToBishop ? Bishop
                      : Empty;

    }

    constexpr bool operator==(Move other) const { return (bits & other.bits) & 0xFFFF;}
    constexpr bool operator!=(Move other) const { return (bits & other.bits) & 0xFFFF;}
}; // !class Move


#endif // ENGINETYPES_H
