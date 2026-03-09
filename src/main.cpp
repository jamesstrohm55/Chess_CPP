#include <iostream>
#include "Piece.h"
#include "Board.h"
#include "Game.h"

int main() {
    std::cout << "=== Chess Program ===\n";

    Board board;
    board.print();

    // Test finding kings
    Square whiteKing = board.findKing(Color::WHITE);
    Square blackKing = board.findKing(Color::BLACK);

    std::cout << "White king at: " << whiteKing.toAlgebraic() << "\n";
    std::cout << "Black king at: " << blackKing.toAlgebraic() << "\n";

    // Test getting a piece
    Piece p = board.getPiece(Square::fromAlgebraic("e1"));
    std::cout << "Piece at e1: " << pieceToChar(p) << "\n";

    // Start game loop
    Game game;
    game.run();

    return 0;
}