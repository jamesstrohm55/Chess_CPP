#ifndef PIECE_H
#define PIECE_H

enum class Color {
    WHITE,
    BLACK,
    NONE
};

enum class PieceType {
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONE
};

struct Piece {
    PieceType type;
    Color color;

    Piece() : type(PieceType::NONE), color(Color::NONE) {} 
    Piece(PieceType t, Color c) : type(t), color(c) {}

    bool isEmpty() const { return type == PieceType::NONE; }
    bool isWhite() const { return color == Color::WHITE; }
    bool isBlack() const { return color == Color::BLACK; }
};

//Get character for console display (uppercase for white, lowercase for black)
inline char pieceToChar(const Piece& p) {
    if (p.isEmpty()) return '.';
    char c;
    switch (p.type) {
        case PieceType::PAWN:  c= 'P'; break;
        case PieceType::KNIGHT:c= 'N'; break;
        case PieceType::BISHOP:c= 'B'; break;
        case PieceType::ROOK:  c= 'R'; break;
        case PieceType::QUEEN: c= 'Q'; break;
        case PieceType::KING:  c= 'K'; break;
        default: return '?';
    }
    return p.isBlack() ? (c + 32) : c; // +32 to convert to lowercase
}

inline Color oppositeColor(Color c) {
    if (c == Color::WHITE) return Color:: BLACK;
    if (c == Color::BLACK) return Color:: WHITE;
    return Color::NONE;
}

#endif // PIECE_H