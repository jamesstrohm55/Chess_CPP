#ifndef GAME_H
#define GAME_H

#include "AI.h"
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
    RESIGNED,
};

enum class GameMode {
    HUMAN_VS_HUMAN,
    HUMAN_VS_CPU
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

    bool isInCheck() const;
    void checkGameOver();
    void resetGame();

    //Set game mode (for GUI)
    void setMode(GameMode mode);

    //Set AI difficulty (for GUI)
    void setDifficulty(Difficulty diff);

    //Set which color the CPU plays (for GUI)
    void setCPUColor(Color color);

    //Called by GUI each frame - makes CPU move if it's CPU's turn
    void handleCPUTurn();

    //Show menu and get user selections (for console mode)
    static void showMenu(GameMode& mode, Difficulty& diff, Color& cpuColor);

    //Score tracking
    int getWhiteWins() const { return whiteWins; }
    int getBlackWins() const { return blackWins; }
    int getDrawCount() const { return drawCount; }

    //Undo last move (in CPU mode, undoes both CPU + player move)
    bool undoLastMove();

    //Cpu turn helpers
    bool isCPUTurn() const;
    std::string getLastMoveString() const;
    const Move* getLastMove() const;
    Color getCPUColor() const { return cpuColor; }
    GameMode getMode() const {return mode; }

private:
    //Game state
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

    //AI
    AI ai;
    GameMode mode = GameMode::HUMAN_VS_HUMAN;
    Color cpuColor = Color::BLACK; // Default CPU plays Black

    void makeCPUMove();

    //Score tracking
    int whiteWins = 0;
    int blackWins = 0;
    int drawCount = 0;
    void updateScore();
};


#endif //GAME_H