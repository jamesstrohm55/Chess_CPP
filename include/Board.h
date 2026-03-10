#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include <array>
#include <string>
#include <vector>

// Represents a square on the board (0-7 for both row and column)
struct Square {
    int row; // 0 = rank 1 (white's back rank), 7 = rank 8
    int col; // 0 = a-file, 7 = h-file

    Square() : row(-1), col(-1) {}
    Square(int r, int c) : row(r), col(c) {}

    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }

    bool operator==(const Square& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Square& other) const {
        return !(*this == other);
    }

    // Convert to algebraic notation (e.g., e4)
    std::string toAlgebraic() const {
        if (!isValid()) return "-";
        char file = 'a' + col;
        char rank  = '1' + row;
        return std::string(1, file) + rank;
    }

    //Create from algebraic notation
    static Square fromAlgebraic(const std::string& s) {
        if (s.length() < 2) return Square();
        int col = s[0] - 'a';
        int row = s[1] - '1';
        Square sq(row, col);
        return sq.isValid() ? sq : Square();
    }
};

struct GameState {
    Color sideToMove = Color::WHITE;
    bool whiteCanCastleKingsside = true;
    bool whiteCanCastleQueensside = true;
    bool blackCanCastleKingsside = true;
    bool blackCanCastleQueensside = true;
    Square enPassantTarget;   // invalid square = no en passant
    int halfmoveClock = 0;    // For 50-move rule
    int fullmoveNumber = 1;   // Starts at 1, increments after Black moves
};

struct Move; //Forward decl to avoid circular include

class Board {
public:
    Board();

    //Reset to starting position
    void reset();

    //Access pieces
    Piece getPiece(const Square& sq) const;
    Piece getPiece(int row, int col) const;
    void setPiece(const Square& sq, const Piece& piece);
    void setPiece(int row, int col, const Piece& piece);

    //Print board to console
    void print() const;

    //Find king of given color
    Square findKing(Color color) const;

    //Game state
    GameState state;

    // Make and undo moves
    void makeMove(Move& move);
    void undoMove(const Move& move);

    //Attack detection
    bool isSquareAttacked(const Square& sq, Color byColor) const;
    bool isInCheck(Color color) const;

private:
    std::array<std::array<Piece, 8>, 8> squares;
    std::vector<GameState> stateHistory;
};

#endif // BOARD_H