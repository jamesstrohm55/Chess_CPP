#ifndef MOVE_H
#define MOVE_H

#include "Piece.h"
#include "Board.h"
#include <string>

struct Move {
    Square from;
    Square to;

    // Flags for special moves
    bool isCapture = false;
    bool isCastling = false;
    bool isEnPassant = false;
    bool isDoublePawnPush = false;

    //For pawn promotion
    PieceType promotionPiece = PieceType::NONE;

    //The piece that was captured (for undo)
    Piece capturedPiece;

    Move() : from(), to() {}

    Move(Square f, Square t) : from(f), to(t) {}

    Move(int fromRow, int fromCol, int toRow, int toCol)
        : from(fromRow, fromCol), to(toRow, toCol) {}

    bool isValid() const {
        return from.isValid() && to.isValid();
    }

    bool isPromotion() const {
        return promotionPiece != PieceType::NONE;
    }

    //Convert to string like "e2e4", "e7e8q" (for promotion)
    std::string toString() const {
        if (!isValid()) return "----";
        std::string s = from.toAlgebraic() + to.toAlgebraic();
        if (isPromotion()) {
            char p;
            switch (promotionPiece) {
                case PieceType::QUEEN: p = 'q'; break;
                case PieceType::ROOK: p = 'r'; break;
                case PieceType::BISHOP: p = 'b'; break;
                case PieceType::KNIGHT: p = 'n'; break;
                default: p = '?';
            }
            s += p;
        }
        return s;
    }
    
    bool operator==(const Move& other) const {
        return from == other.from && to == other.to && promotionPiece == other.promotionPiece;
    }
};

#endif // MOVE_H