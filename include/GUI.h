#ifndef GUI_H
#define GUI_H

#include "Game.h"
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>

class ChessGUI {
public:
    ChessGUI(Game& game);
    ~ChessGUI();

    bool init(int windowSize = 640);
    void run();
    void shutdown();

private:
    Game& game;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    int windowSize;
    int squareSize;

    //Piece textures keyed by "wP", "bK", etc.
    std::unordered_map<std::string, SDL_Texture*> pieceTextures;

    //interaction state
    Square selectedSquare;
    std::vector<Square> highlightedSquares;

    //Promotion UI
    bool awaitingPromotion = false;
    Move pendingPromotionMove;

    //Rendering
    void render();
    void drawBoard();
    void drawPieces();
    void drawHighlights();
    void drawPromotionDialog();

    //Textures
    bool loadPieceTextures(const std::string& assetPath);
    std::string pieceTextureKey(const Piece& p) const;

    //Coordinate conversion
    Square pixelToSquare(int x, int y) const;
    SDL_Rect squareToRect(const Square& sq) const;

    //Input
    void handleMouseClick(int x, int y);
    void selectPiece(const Square& sq);
    void clearSelection();
};

#endif // GUI_H