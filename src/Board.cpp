#include "Board.h"
#include <iostream>

Board::Board() {
    reset();
}

void Board::reset() {
    //Clear board
    for (int row =0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            squares[row][col] = Piece();
        }
    }

    //Set up white pices (row 0 and 1
    squares[0][0] = Piece(PieceType::ROOK, Color::WHITE);
    squares[0][1] = Piece(PieceType::KNIGHT, Color::WHITE);
    squares[0][2] = Piece(PieceType::BISHOP, Color::WHITE);
    squares[0][3] = Piece(PieceType::QUEEN, Color::WHITE);
    squares[0][4] = Piece(PieceType::KING, Color::WHITE);
    squares[0][5] = Piece(PieceType::BISHOP, Color::WHITE);
    squares[0][6] = Piece(PieceType::KNIGHT, Color::WHITE);
    squares[0][7] = Piece(PieceType::ROOK, Color::WHITE);

    for (int col = 0; col < 8; ++col) {
        squares[1][col] = Piece(PieceType::PAWN, Color::WHITE);
    }

    //Set up black pieces (row 7 and 6)
    squares[7][0] = Piece(PieceType::ROOK, Color::BLACK);
    squares[7][1] = Piece(PieceType::KNIGHT, Color::BLACK);
    squares[7][2] = Piece(PieceType::BISHOP, Color::BLACK);
    squares[7][3] = Piece(PieceType::QUEEN, Color::BLACK);
    squares[7][4] = Piece(PieceType::KING, Color::BLACK);
    squares[7][5] = Piece(PieceType::BISHOP, Color::BLACK);
    squares[7][6] = Piece(PieceType::KNIGHT, Color::BLACK);
    squares[7][7] = Piece(PieceType::ROOK, Color::BLACK);

    for (int col = 0; col < 8; ++col) {
        squares[6][col] = Piece(PieceType::PAWN, Color::BLACK);
    }
}

Piece Board::getPiece(const Square& sq) const {
    if (!sq.isValid()) return Piece();
    return squares[sq.row][sq.col];
}

Piece Board::getPiece(int row, int col) const {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return Piece();
    return squares[row][col];
}

void Board::setPiece(const Square& sq, const Piece& piece) {
    if (sq.isValid()) {
        squares[sq.row][sq.col] = piece;
    }
}

void Board::setPiece(int row, int col, const Piece& piece) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        squares[row][col] = piece;
    }
}

void Board::print() const {
    std::cout << "\n";
    for (int row = 7; row >= 0; --row) {
        std::cout << (row + 1) << " ";
        for (int col = 0; col < 8; ++col) {
            std::cout << pieceToChar(squares[row][col]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

Square Board::findKing(Color color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece& p = squares[row][col];
            if (p.type == PieceType::KING && p.color == color) {
                return Square(row, col);
            }
        }
    }
    return Square(); // Invalid square if not found
}