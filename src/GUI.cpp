#include "GUI.h"
#include <iostream>

ChessGUI::ChessGUI(Game& game) : game(game) {}

ChessGUI::~ChessGUI() {
    shutdown();
}

bool ChessGUI::init(int size) {
    windowSize = size;
    squareSize = size / 8;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    id (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("Chess",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowSize, windowSize, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!loadPieceTextures("assets/pieces/")) {
        std::cerr << "Failed to load piece textures\n";
    }

    return true;
}

bool ChessGUI::loadPieceTextures(const std::string& assetPath) {
    const char* pieces[] = {"wP", "wN", "wB", "wR", "wQ", "wK",
                            "bP", "bN", "bB", "bR", "bQ", "bK"};

    bool allLoaded = true;

    for (const char* name : pieces) {
        std::string path = assetPath + name + ".svg";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Failed to load " << path << ": " << IMG_GetError() << "\n";
            allLoaded = false;
            continue;
        }
        SDL_texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (tex) {
            pieceTextures[name] = tex;
        }
    }
    return allLoaded;
}

