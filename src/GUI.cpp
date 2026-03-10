#include "GUI.h"
#include <iostream>

ChessGUI::ChessGUI(Game &game) : game(game) {}

ChessGUI::~ChessGUI()
{
    shutdown();
}

bool ChessGUI::init(int size)
{
    windowSize = size;
    squareSize = size / 8;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << "\n";
        return false;
    }

    if (TTF_Init() < 0)
    {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("Chess",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              windowSize, windowSize + panelHeight, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!loadPieceTextures("assets/pieces/"))
    {
        std::cerr << "Failed to load piece textures\n";
    }

    font = TTF_OpenFont("C:/Windows/Fonts/segoeui.ttf", 24);
    titleFont = TTF_OpenFont("C:/Windows/Fonts/segoeuib.ttf", 48);
    if (!font || !titleFont)
    {
        font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
        titleFont = TTF_OpenFont("C:/Windows/Fonts/arialbd.ttf", 48);
    }
    if (!font)
    {
        std::cerr << "Failed to load font\n";
        return false;
    }

    return true;
}

bool ChessGUI::loadPieceTextures(const std::string &assetPath)
{
    const char *pieces[] = {"wP", "wN", "wB", "wR", "wQ", "wK",
                            "bP", "bN", "bB", "bR", "bQ", "bK"};

    bool allLoaded = true;

    for (const char *name : pieces)
    {
        std::string path = assetPath + name + ".svg";
        SDL_Surface *surface = IMG_Load(path.c_str());
        if (!surface)
        {
            std::cerr << "Failed to load " << path << ": " << IMG_GetError() << "\n";
            allLoaded = false;
            continue;
        }
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (tex)
        {
            pieceTextures[name] = tex;
        }
    }
    return allLoaded;
}

std::string ChessGUI::pieceTextureKey(const Piece &p) const
{
    if (p.isEmpty())
        return "";
    std::string key;
    key += p.isWhite() ? 'w' : 'b';
    switch (p.type)
    {
    case PieceType::PAWN:
        key += 'P';
        break;
    case PieceType::KNIGHT:
        key += 'N';
        break;
    case PieceType::BISHOP:
        key += 'B';
        break;
    case PieceType::ROOK:
        key += 'R';
        break;
    case PieceType::QUEEN:
        key += 'Q';
        break;
    case PieceType::KING:
        key += 'K';
        break;
    default:
        return "";
    }
    return key;
}

Square ChessGUI::pixelToSquare(int x, int y) const
{
    int col = x / squareSize;
    int row = 7 - (y / squareSize); // Invert y to match chess board orientation
    return Square(row, col);
}

SDL_Rect ChessGUI::squareToRect(const Square &sq) const
{
    SDL_Rect rect;
    rect.x = sq.col * squareSize;
    rect.y = (7 - sq.row) * squareSize; // Invert y
    rect.w = squareSize;
    rect.h = squareSize;
    return rect;
}

// Rendering
void ChessGUI::render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawBoard();
    drawHighlights();
    drawPieces();
    drawStatusPanel();

    if (awaitingPromotion)
    {
        drawPromotionDialog();
    }
    else if (game.getResult() != GameResult::IN_PROGRESS)
    {
        drawGameOverOverlay();
    }
    else if (game.isInCheck())
    {
        drawCheckBanner();
    }

    SDL_RenderPresent(renderer);
}

void ChessGUI::drawBoard()
{
    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            SDL_Rect rect = squareToRect(Square(row, col));
            bool isLight = (row + col) % 2 != 0;
            if (isLight)
            {
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255); // Light square
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255); // Dark square
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // Highlight selected square
    if (selectedSquare.isValid())
    {
        SDL_Rect rect = squareToRect(selectedSquare);
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 128); // Yellow highlight
        SDL_RenderFillRect(renderer, &rect);
    }
}

void ChessGUI::drawPieces()
{
    const Board &board = game.getBoard();
    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            Piece p = board.getPiece(row, col);
            if (p.isEmpty())
                continue;

            std::string key = pieceTextureKey(p);
            auto it = pieceTextures.find(key);
            if (it != pieceTextures.end())
            {
                SDL_Rect rect = squareToRect(Square(row, col));
                // Add small padding
                int pad = squareSize / 10;
                rect.x += pad;
                rect.y += pad;
                rect.w -= 2 * pad;
                rect.h -= 2 * pad;
                SDL_RenderCopy(renderer, it->second, nullptr, &rect);
            }
        }
    }
}

void ChessGUI::drawHighlights()
{
    for (const Square &sq : highlightedSquares)
    {
        SDL_Rect rect = squareToRect(sq);
        int cx = rect.x + rect.w / 2;
        int cy = rect.y + rect.h / 2;

        // Check if this square has a piece (capture highlight)
        Piece target = game.getBoard().getPiece(sq);
        if (!target.isEmpty())
        {
            // Draw a red-tinted overlay for captures
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 80);
            SDL_RenderFillRect(renderer, &rect);
        }
        else
        {
            // Draw a small circle (as a filled rect approximation) for normal moves
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
            int dotSize = squareSize / 4;
            SDL_Rect dot = {cx - dotSize / 2, cy - dotSize / 2, dotSize, dotSize};
            SDL_RenderFillRect(renderer, &dot);
        }
    }
}

void ChessGUI::drawStatusPanel()
{
    // Panel background
    SDL_Rect panel = {0, windowSize, windowSize, panelHeight};
    SDL_SetRenderDrawColor(renderer, 48, 44, 40, 255);
    SDL_RenderFillRect(renderer, &panel);

    // Sparator line
    SDL_SetRenderDrawColor(renderer, 120, 80, 50, 255);
    SDL_RenderDrawLine(renderer, 0, windowSize, windowSize, windowSize);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255};

    // Score display
    std::string scoreText;
    if (game.getMode() == GameMode::HUMAN_VS_CPU)
    {
        int playerWins = (game.getCPUColor() == Color::BLACK) ? game.getWhiteWins() : game.getBlackWins();
        int cpuWins = (game.getCPUColor() == Color::BLACK) ? game.getBlackWins() : game.getWhiteWins();
        scoreText = "You: " + std::to_string(playerWins) +
                    " CPU: " + std::to_string(cpuWins) +
                    " Draws: " + std::to_string(game.getDrawCount());
    }
    else
    {
        scoreText = "White: " + std::to_string(game.getWhiteWins()) +
                    " Black: " + std::to_string(game.getBlackWins()) +
                    " Draws: " + std::to_string(game.getDrawCount());
    }

    drawTextCentered(scoreText, windowSize / 2, windowSize + 15, gold, font);

    // Status message
    if (!statusMessage.empty())
    {
        drawTextCentered(statusMessage, windowSize / 2, windowSize + 42, white, font);
    }
}

void ChessGUI::drawPromotionDialog()
{
    // Draw a vertical strip of 45 promotion pieces over the target column
    Color color = game.getBoard().state.sideToMove;
    // sideToMove has already be toggled after makeMove
    // so promotion pieces should be of the opposite color
    Color promoColor = oppositeColor(color);

    PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT};

    int col = pendingPromotionMove.to.col;
    int startY = (promoColor == Color::WHITE) ? 0 : squareSize * 4;

    // Background overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_Rect overlay = {0, 0, windowSize, windowSize};
    SDL_RenderFillRect(renderer, &overlay);

    for (int i = 0; i < 4; ++i)
    {
        SDL_Rect rect = {col * squareSize, startY + i * squareSize,
                         squareSize, squareSize};

        // White background for visibility
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);

        // Draw piece
        Piece promoPiece(promos[i], promoColor);
        std::string key = pieceTextureKey(promoPiece);
        auto it = pieceTextures.find(key);
        if (it != pieceTextures.end())
        {
            int pad = squareSize / 10;
            SDL_Rect pieceRect = {rect.x + pad, rect.y + pad,
                                  rect.w - 2 * pad, rect.h - 2 * pad};
            SDL_RenderCopy(renderer, it->second, nullptr, &pieceRect);
        }
    }
}

// ---Input handling---
void ChessGUI::selectPiece(const Square &sq)
{
    selectedSquare = sq;
    highlightedSquares.clear();

    // Find all legal destinations for this piece
    for (const auto &move : game.getLegalMoves())
    {
        if (move.from == sq)
        {
            highlightedSquares.push_back(move.to);
        }
    }
}

void ChessGUI::clearSelection()
{
    selectedSquare = Square();
    highlightedSquares.clear();
}

void ChessGUI::handleMouseClick(int x, int y)
{
    if (game.getResult() != GameResult::IN_PROGRESS)
        return;

    // Handle promotion dialog click
    if (awaitingPromotion)
    {
        Color promoColor = oppositeColor(game.getBoard().state.sideToMove);
        int col = pendingPromotionMove.to.col;
        int startY = (promoColor == Color::WHITE) ? 0 : squareSize * 4;

        int clickCol = x / squareSize;
        int clickIdx = (y - startY) / squareSize;

        if (clickCol == col && clickIdx >= 0 && clickIdx < 4)
        {
            PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK,
                                  PieceType::BISHOP, PieceType::KNIGHT};
            pendingPromotionMove.promotionPiece = promos[clickIdx];
            game.tryMakeMove(pendingPromotionMove);
            const char *promoNames[] = {"Queen", "Rook", "Bishop", "Knight"};
            std::string promoStr = promoNames[clickIdx];
            if (game.getResult() != GameResult::IN_PROGRESS)
                statusMessage = "You promoted to " + promoStr + ". Game Over!";
            else if (game.isInCheck())
                statusMessage = "You promoted to " + promoStr + ". Check!";
            else
                statusMessage = "You promoted to " + promoStr + ". Your turn.";
            awaitingPromotion = false;
            clearSelection();
        }
        return;
    }

    Square clicked = pixelToSquare(x, y);
    if (!clicked.isValid())
        return;

    const Board &board = game.getBoard();
    Piece clickedPiece = board.getPiece(clicked);

    if (selectedSquare.isValid())
    {
        // Check if clicked square is a legal destination
        bool isHighlighted = false;
        for (const Square &sq : highlightedSquares)
        {
            if (sq == clicked)
            {
                isHighlighted = true;
                break;
            }
        }

        if (isHighlighted)
        {
            Piece movingPiece = board.getPiece(selectedSquare);
            int promoRank = (movingPiece.color == Color::WHITE) ? 7 : 0;
            if (movingPiece.type == PieceType::PAWN && clicked.row == promoRank)
            {
                pendingPromotionMove = Move(selectedSquare, clicked);
                pendingPromotionMove.isCapture = !board.getPiece(clicked).isEmpty();
                awaitingPromotion = true;
                return;
            }

            Move move(selectedSquare, clicked);
            game.tryMakeMove(move);
            if (game.getResult() != GameResult::IN_PROGRESS)
                statusMessage = "You played: " + move.toString() + ". Game Over!";
            else if (game.isInCheck())
                statusMessage = "You played: " + move.toString() + ". Check!";
            else
                statusMessage = "You played: " + move.toString() + ". CPU's turn.";
            clearSelection();
        }
        else if (!clickedPiece.isEmpty() &&
                 clickedPiece.color == board.state.sideToMove)
        {
            selectPiece(clicked);
        }
        else
        {
            clearSelection();
        }
    }
    else
    {
        if (!clickedPiece.isEmpty() &&
            clickedPiece.color == board.state.sideToMove)
        {
            selectPiece(clicked);
        }
    }
}

// ---Main loop---
void ChessGUI::run()
{
    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (guiState == GUIState::PLAYING)
                    {
                        if (game.getResult() != GameResult::IN_PROGRESS)
                        {
                            handleGameOverClick(event.button.x, event.button.y);
                        }
                        else
                        {
                            handleMouseClick(event.button.x, event.button.y);
                        }
                    }
                    else
                    {
                        handleMenuClick(event.button.x, event.button.y);
                    }
                }
            }
        }

        if (guiState == GUIState::PLAYING)
        {
            if (game.isCPUTurn())
            {
                statusMessage = "CPU is thinking...";
                render();
                game.handleCPUTurn();
                std::string lastMove = game.getLastMoveString();
                if (game.getResult() != GameResult::IN_PROGRESS)
                {
                    if (game.getResult() == GameResult::WHITE_WINS || game.getResult() == GameResult::BLACK_WINS)
                        statusMessage = "CPU played: " + lastMove + ". Checkmate!";
                    else
                        statusMessage = "CPU played: " + lastMove + ". It's a draw!";
                }
                else if (game.isInCheck())
                {
                    statusMessage = "CPU played: " + lastMove + ". Check!";
                }
                else if (!lastMove.empty())
                {
                    statusMessage = "CPU played: " + lastMove + ". Your turn.";
                }
            }
            render();
        }
        else
        {
            renderMenu();
        }
    }
}

void ChessGUI::shutdown()
{
    for (auto &pair : pieceTextures)
    {
        if (pair.second)
            SDL_DestroyTexture(pair.second);
    }
    pieceTextures.clear();

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    if (font)
    {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if (titleFont)
    {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void ChessGUI::drawTextCentered(const std::string &text, int centerX, int centerY, SDL_Color color, TTF_Font *f)
{
    SDL_Surface *surface = TTF_RenderText_Blended(f, text.c_str(), color);
    if (!surface)
        return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {centerX - surface->w / 2, centerY - surface->h / 2,
                     surface->w, surface->h};
    SDL_FreeSurface(surface);
    if (texture)
    {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_DestroyTexture(texture);
    }
}

bool ChessGUI::isInsideRect(int x, int y, const SDL_Rect &rect) const
{
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

void ChessGUI::renderMenu()
{
    SDL_SetRenderDrawColor(renderer, 48, 44, 40, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255};

    int btnW = 300, btnH = 60;
    int btnX = (windowSize - btnW) / 2;

    switch (guiState)
    {
    case GUIState::MENU_MODE:
    {
        drawTextCentered("CHESS", windowSize / 2, 140, gold, titleFont);

        SDL_Rect btn1 = {btnX, 280, btnW, btnH};
        SDL_Rect btn2 = {btnX, 370, btnW, btnH};

        SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
        SDL_RenderFillRect(renderer, &btn1);
        SDL_RenderFillRect(renderer, &btn2);

        SDL_SetRenderDrawColor(renderer, 120, 80, 50, 255);
        SDL_RenderDrawRect(renderer, &btn1);
        SDL_RenderDrawRect(renderer, &btn2);

        drawTextCentered("Play vs Human", windowSize / 2, btn1.y + btnH / 2, white, font);
        drawTextCentered("Play vs Computer", windowSize / 2, btn2.y + btnH / 2, white, font);
        break;
    }
    case GUIState::MENU_COLOR:
    {
        drawTextCentered("Play as ...", windowSize / 2, 140, gold, titleFont);

        SDL_Rect btn1 = {btnX, 280, btnW, btnH};
        SDL_Rect btn2 = {btnX, 370, btnW, btnH};

        SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
        SDL_RenderFillRect(renderer, &btn1);
        SDL_RenderFillRect(renderer, &btn2);

        SDL_SetRenderDrawColor(renderer, 120, 80, 50, 255);
        SDL_RenderDrawRect(renderer, &btn1);
        SDL_RenderDrawRect(renderer, &btn2);

        drawTextCentered("White", windowSize / 2, btn1.y + btnH / 2, white, font);
        drawTextCentered("Black", windowSize / 2, btn2.y + btnH / 2, white, font);
        break;
    }
    case GUIState::MENU_DIFFICULTY:
    {
        drawTextCentered("Select Difficulty", windowSize / 2, 140, gold, titleFont);

        SDL_Rect btn1 = {btnX, 240, btnW, btnH};
        SDL_Rect btn2 = {btnX, 330, btnW, btnH};
        SDL_Rect btn3 = {btnX, 420, btnW, btnH};

        SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
        SDL_RenderFillRect(renderer, &btn1);
        SDL_RenderFillRect(renderer, &btn2);
        SDL_RenderFillRect(renderer, &btn3);

        SDL_SetRenderDrawColor(renderer, 120, 80, 50, 255);
        SDL_RenderDrawRect(renderer, &btn1);
        SDL_RenderDrawRect(renderer, &btn2);
        SDL_RenderDrawRect(renderer, &btn3);

        drawTextCentered("Easy", windowSize / 2, btn1.y + btnH / 2, white, font);
        drawTextCentered("Medium", windowSize / 2, btn2.y + btnH / 2, white, font);
        drawTextCentered("Hard", windowSize / 2, btn3.y + btnH / 2, white, font);
        break;
    }
    default:
        break;
    }

    SDL_RenderPresent(renderer);
}

void ChessGUI::handleMenuClick(int x, int y)
{
    int btnW = 300, btnH = 60;
    int btnX = (windowSize - btnW) / 2;

    switch (guiState)
    {
    case GUIState::MENU_MODE:
    {
        SDL_Rect btn1 = {btnX, 280, btnW, btnH};
        SDL_Rect btn2 = {btnX, 370, btnW, btnH};

        if (isInsideRect(x, y, btn1))
        {
            game.setMode(GameMode::HUMAN_VS_HUMAN);
            guiState = GUIState::PLAYING;
            statusMessage = "White's turn. Click a piece to move.";
        }
        else if (isInsideRect(x, y, btn2))
        {
            game.setMode(GameMode::HUMAN_VS_CPU);
            guiState = GUIState::MENU_COLOR;
        }
        break;
    }
    case GUIState::MENU_COLOR:
    {
        SDL_Rect btn1 = {btnX, 280, btnW, btnH};
        SDL_Rect btn2 = {btnX, 370, btnW, btnH};

        if (isInsideRect(x, y, btn1))
        {
            game.setCPUColor(Color::BLACK);
            guiState = GUIState::MENU_DIFFICULTY;
        }
        else if (isInsideRect(x, y, btn2))
        {
            game.setCPUColor(Color::WHITE);
            guiState = GUIState::MENU_DIFFICULTY;
        }
        break;
    }
    case GUIState::MENU_DIFFICULTY:
    {
        SDL_Rect btn1 = {btnX, 240, btnW, btnH};
        SDL_Rect btn2 = {btnX, 330, btnW, btnH};
        SDL_Rect btn3 = {btnX, 420, btnW, btnH};

        if (isInsideRect(x, y, btn1))
        {
            game.setDifficulty(Difficulty::EASY);
            guiState = GUIState::PLAYING;
            statusMessage = "You are playing against an Easy CPU. White's turn.";
        }
        else if (isInsideRect(x, y, btn2))
        {
            game.setDifficulty(Difficulty::MEDIUM);
            guiState = GUIState::PLAYING;
            statusMessage = "You are playing against a Medium CPU. White's turn.";
        }
        else if (isInsideRect(x, y, btn3))
        {
            game.setDifficulty(Difficulty::HARD);
            guiState = GUIState::PLAYING;
            statusMessage = "You are playing against a Hard CPU. White's turn.";
        }
        break;
    }
    default:
        break;
    }
}

void ChessGUI::drawCheckBanner()
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 180);
    SDL_Rect banner = {0, windowSize / 2 - 25, windowSize, 50};
    SDL_RenderFillRect(renderer, &banner);

    SDL_Color white = {255, 255, 255, 255};
    drawTextCentered("Check!", windowSize / 2, windowSize / 2, white, titleFont);
}

void ChessGUI::drawGameOverOverlay()
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect overlay = {0, 0, windowSize, windowSize};
    SDL_RenderFillRect(renderer, &overlay);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gold = {255, 215, 0, 255};

    std::string resultText;
    switch (game.getResult())
    {
    case GameResult::WHITE_WINS:
        resultText = "White Wins!";
        break;
    case GameResult::BLACK_WINS:
        resultText = "Black Wins!";
        break;
    case GameResult::DRAW_STALEMATE:
        resultText = "Stalemate! Draw!";
        break;
    case GameResult::DRAW_50_MOVE:
        resultText = "50-Move Rule! Draw!";
        break;
    case GameResult::DRAW_INSUFFICIENT:
        resultText = "Insufficient Material! Draw!";
        break;
    case GameResult::RESIGNED:
        resultText = "Resignation!";
        break;
    default:
        resultText = "Game Over!";
        break;
    }

    drawTextCentered(resultText, windowSize / 2, 180, gold, titleFont);

    int btnW = 220, btnH = 50;
    int btnX = (windowSize - btnW) / 2;

    SDL_Rect btnPlayAgain = {btnX, 300, btnW, btnH};
    SDL_Rect btnMainMenu = {btnX, 370, btnW, btnH};
    SDL_Rect btnQuit = {btnX, 440, btnW, btnH};

    SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
    SDL_RenderFillRect(renderer, &btnPlayAgain);
    SDL_RenderFillRect(renderer, &btnMainMenu);
    SDL_RenderFillRect(renderer, &btnQuit);

    SDL_SetRenderDrawColor(renderer, 120, 80, 50, 255);
    SDL_RenderDrawRect(renderer, &btnPlayAgain);
    SDL_RenderDrawRect(renderer, &btnMainMenu);
    SDL_RenderDrawRect(renderer, &btnQuit);

    drawTextCentered("Play Again", windowSize / 2, btnPlayAgain.y + btnH / 2, white, font);
    drawTextCentered("Main Menu", windowSize / 2, btnMainMenu.y + btnH / 2, white, font);
    drawTextCentered("Quit", windowSize / 2, btnQuit.y + btnH / 2, white, font);
}

void ChessGUI::handleGameOverClick(int x, int y)
{
    int btnW = 220, btnH = 50;
    int btnX = (windowSize - btnW) / 2;

    SDL_Rect btnPlayAgain = {btnX, 300, btnW, btnH};
    SDL_Rect btnMainMenu = {btnX, 370, btnW, btnH};
    SDL_Rect btnQuit = {btnX, 440, btnW, btnH};

    if (isInsideRect(x, y, btnPlayAgain))
    {
        game.resetGame();
        clearSelection();
    }
    else if (isInsideRect(x, y, btnMainMenu))
    {
        game.resetGame();
        clearSelection();
        guiState = GUIState::MENU_MODE;
    }
    else if (isInsideRect(x, y, btnQuit))
    {
        SDL_Event quit;
        quit.type = SDL_QUIT;
        SDL_PushEvent(&quit);
    }
}