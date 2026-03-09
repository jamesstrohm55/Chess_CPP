#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "Board.h"
#include "Move.h"
#include <vector>

class MoveGenerator {
    public:
        //Generate all legal moves for the side to move
        std::vector<Move> generateLegalMoves(Board& board) const;

        //Generate all pseudo-legal moves (may leave king in check)
        std::vector<Move> generatePseudoLegalMoves(Board& board) const;

        //Game ending queries
        bool isCheckmate(Board& board) const;
        bool isStalemate(Board& board) const;
        bool isDraw(const Board& board) const; //50 move rule or insufficient material

    private:
        void generatePawnMoves(const Board& board, const Square& from, Color color,
                                std::vector<Move>& moves) const;
        void generateKnightMoves(const Board& board, const Square& from, Color color,
                                std::vector<Move>& moves) const;
        void generateSlidingMoves(const Board& board, const Square& from, Color color,
                                const int directions[][2], int numDirections,
                                std::vector<Move>& moves) const;
        void generateKingMoves(const Board& board, const Square& from, Color color,
                                std::vector<Move>& moves) const;
        void generateCastlingMoves(const Board& board, Color color,
                                std::vector<Move>& moves) const;

        //Helper: adds pawn move, expanding into 4 promotion moves if on last rank
        void addPawnMove(const Square& from, const Square& to, Color color,
                        bool isCapture, std::vector<Move>& moves) const;
};

#endif //MOVEGENERATOR_H