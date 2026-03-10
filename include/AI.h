#ifndef AI_H
#define AI_H

#include "Board.h"
#include "Move.h"
#include "MoveGenerator.h"

enum class Difficulty {
    EASY, // depth 2
    MEDIUM, // depth 4
    HARD  // depth 6
};

class AI {
public:
    AI(Difficulty diff = Difficulty::MEDIUM);

    //Picks the best move for the current board state
    Move findBestMove(Board& board);

    void setDifficulty(Difficulty diff);

private:
    Difficulty difficulty;
    MoveGenerator moveGen;

    static const int MAX_DEPTH = 20;
    Move killerMoves[MAX_DEPTH][2];

    int getSearchDepth() const;

    //Minimax with alpha-beta pruning
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizing);

    //Quiescence search - resolves captures to avoid horizon effect
    int quiescence(Board& board, int alpha, int beta, bool maximizing);

    //Static evaluation function for board positions
    int evaluate(const Board& board) const;

    //Evaluation helpers
    int evaluateKingSafety(const Board& board, Color color) const;
    int evaluatePawnStructure(const Board& board) const;
    int evaluateBishopPair(const Board& board) const;

    //Material value of a piece
    int pieceValue(PieceType type) const;

    //Piece square table bonus for positional evaluation
    int positionBonus(PieceType type, Color color, int row, int col) const;

    int scoreMove(const Move& move, const Board& board, int depth = -1) const; //For move ordering

    //Killer move heuristic
    bool isKillerMove(const Move& move, int depth) const;
    void storeKillerMove(const Move& move, int depth);
    void clearKillerMoves();
};

#endif // AI_H