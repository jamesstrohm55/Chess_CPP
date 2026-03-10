#ifndef GUI_H
#define GUI_H

#include "Game.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

enum class GUIState
{
    MENU_MODE,
    MENU_COLOR,
    MENU_DIFFICULTY,
    PLAYING
};

class ChessGUI
{
public:
    ChessGUI(Game &game);
    ~ChessGUI();

    bool init(int windowSize = 640);
    void run();
    void shutdown();

private:
    Game &game;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    int windowSize;
    int squareSize;

    // Piece textures keyed by "wP", "bK", etc.
    std::unordered_map<std::string, SDL_Texture *> pieceTextures;

    // interaction state
    Square selectedSquare;
    std::vector<Square> highlightedSquares;

    // Promotion UI
    bool awaitingPromotion = false;
    Move pendingPromotionMove;

    // Menu state
    GUIState guiState = GUIState::MENU_MODE;
    TTF_Font *font = nullptr;
    TTF_Font *titleFont = nullptr;

    // Rendering
    void render();
    void drawBoard();
    void drawPieces();
    void drawHighlights();
    void drawPromotionDialog();
    void drawCheckBanner();
    void drawGameOverOverlay();
    void handleGameOverClick(int x, int y);

    // Menu rendering
    void renderMenu();
    void handleMenuClick(int x, int y);
    void drawTextCentered(const std::string &text, int centerX, int centerY,
                          SDL_Color color, TTF_Font *f);
    bool isInsideRect(int x, int y, const SDL_Rect &rect) const;

    // Textures
    bool loadPieceTextures(const std::string &assetPath);
    std::string pieceTextureKey(const Piece &p) const;

    // Coordinate conversion
    Square pixelToSquare(int x, int y) const;
    SDL_Rect squareToRect(const Square &sq) const;

    // Input
    void handleMouseClick(int x, int y);
    void selectPiece(const Square &sq);
    void clearSelection();

    // Status panel
    std::string statusMessage;
    int panelHeight = 60;
    void drawStatusPanel();
    SDL_Rect undoButtonRect = {0, 0, 0, 0};
    void handleUndoClick();

    // Main loop
    bool running = true;
    void runOneFrame();
#ifdef __EMSCRIPTEN__
    static void mainLoopCallback(void *arg);
#endif
};

#endif // GUI_H