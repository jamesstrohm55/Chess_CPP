#ifndef GAME_H
#define GAME_H

#include "Board.h"
#include "Move.h"
#include "MoveGenerator.h"
#include <string>
#include <vector>

enum class GameResult {
    IN_PROGRESS,
    WHITE_WINS,
    BLACK_WINS,
    DRAW_STALEMATE,
    DRAW_50_MOVE,
    DRAW_INSUFFICIENT,
    RESIGNED
};

class Game {
public:
    Game();

    //Main console game loop
    void run();

    //Try to apply a move - returns true if legal and applied, false if illegal
    bool tryMakeMove(const Move& move);

    //Getters (for GUI)
    const Board& getBoard() const { return board; }
    Board& getBoard() { return board; }
    GameResult getResult() const { return result; }
    const std::vector<Move>& getLegalMoves() const { return currentLegalMoves; }

    //Refresh legal moves list
    void updateLegalMoves();

private:
    Board board;
    MoveGenerator moveGen;
    GameResult result = GameResult::IN_PROGRESS;
    std::vector<Move> currentLegalMoves;
    std::vector<Move> moveHistory; // For undo functionality

    //Input parsing
    Move parseInput(const std::string& input) const;
    Move parseUCI(const std::string& input) const;
    Move parseAlgebraic(const std::string& input) const;

    //Display
    void printStatus() const;
};


#endif //GAME_H