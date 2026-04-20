#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cctype>

Game::Game()
{
    board.reset();
    updateLegalMoves();
}

void Game::updateLegalMoves()
{
    currentLegalMoves = moveGen.generateLegalMoves(board);
}

bool Game::isInCheck() const{
    return board.isInCheck(board.state.sideToMove);
}

void Game::checkGameOver()
{
    if (result != GameResult::IN_PROGRESS)
    return; // Already determined

    if (moveGen.isCheckmate(board))
    {
        result = (board.state.sideToMove == Color::WHITE)
                        ? GameResult::BLACK_WINS
                        : GameResult::WHITE_WINS;
    }
    else if (moveGen.isStalemate(board))
    {
        result = GameResult::DRAW_STALEMATE;
    }
    else if (moveGen.isDraw(board))
    {
        result = GameResult::DRAW_50_MOVE;
    }

    if (result != GameResult::IN_PROGRESS) {
        updateScore();
    }
}

void Game::resetGame()
{
    board.reset();
    result = GameResult::IN_PROGRESS;
    moveHistory.clear();
    updateLegalMoves();
}

bool Game::undoLastMove()
{
    if (moveHistory.empty())
        return false;

    // Undo last move
    board.undoMove(moveHistory.back());
    moveHistory.pop_back();

    // In CPU mode, also undo the CPU's move so it's the player's turn again
    if (mode == GameMode::HUMAN_VS_CPU && !moveHistory.empty())
    {
        board.undoMove(moveHistory.back());
        moveHistory.pop_back();
    }

    result = GameResult::IN_PROGRESS;
    updateLegalMoves();
    return true;
}

bool Game::tryMakeMove(const Move &move)
{
    // Find matching move in legal moves
    for (auto &legal : currentLegalMoves)
    {
        if (legal.from == move.from && legal.to == move.to &&
            legal.promotionPiece == move.promotionPiece)
        {
            Move m = legal; // copy so makeMove can modify capturedPiece
            board.makeMove(m);
            moveHistory.push_back(m);
            updateLegalMoves();
            checkGameOver();
            return true;
        }
    }
    return false; // Not found in legal moves
}

void Game::printGameResult() const
{
    if (result == GameResult::IN_PROGRESS) return;
    board.print();
    if (result == GameResult::WHITE_WINS)
        std::cout << "Checkmate! White wins.\n";
    else if (result == GameResult::BLACK_WINS)
        std::cout << "Checkmate! Black wins.\n";
    else if (result == GameResult::DRAW_STALEMATE)
        std::cout << "Stalemate! It's a draw.\n";
    else if (result == GameResult::DRAW_50_MOVE)
        std::cout << "Draw by 50-move rule.\n";
    else if (result == GameResult::DRAW_INSUFFICIENT)
        std::cout << "Draw by insufficient material.\n";
}

void Game::printStatus() const
{
    std::string side = (board.state.sideToMove == Color::WHITE) ? "White" : "Black";
    if (result == GameResult::IN_PROGRESS)
    {
        std::cout << side << "to move.";
        if (board.isInCheck(board.state.sideToMove))
        {
            std::cout << " Check!";
        }
        std::cout << "\n";
    }
}

// --- Main Game Loop ---
void Game::run()
{
    std::string input;

    while (result == GameResult::IN_PROGRESS)
    {
        board.print();
        printStatus();

        // CPU's turn
        if (mode == GameMode::HUMAN_VS_CPU &&
            board.state.sideToMove == cpuColor)
        {
            makeCPUMove();
            printGameResult();
            continue;
        }

        std::cout << "> ";

        if (!std::getline(std::cin, input))
            break;

        // Trim whitespace
        while (!input.empty() && input.back() == ' ')
            input.pop_back();
        while (!input.empty() && input.front() == ' ')
            input.erase(input.begin());
        if (input.empty())
            continue;

        // Commands
        if (input == "quit" || input == "exit")
            break;

        if (input == "help")
        {
            std::cout << "Commands: quit, resign, undo, moves, help\n";
            std::cout << "Enter moves in UCI format (e.g., e2e4) or algebraic notation (e.g., Nf3, O-O)\n";
            continue;
        }

        if (input == "resign")
        {
            result = GameResult::RESIGNED;
            std::string winner = (board.state.sideToMove == Color::WHITE) ? "Black" : "White";
            std::cout << winner << " wins by resignation.\n";
            break;
        }

        if (input == "undo")
        {
            if (moveHistory.empty())
            {
                std::cout << "Nothing to undo.\n";
            }
            else
            {
                board.undoMove(moveHistory.back());
                moveHistory.pop_back();
                // In CPU mode, also undo the CPU's move
                if (mode == GameMode::HUMAN_VS_CPU && !moveHistory.empty())
                {
                    board.undoMove(moveHistory.back());
                    moveHistory.pop_back();
                }
                updateLegalMoves();
                std::cout << "Move undone.\n";
            }
            continue;
        }

        if (input == "moves")
        {
            std::cout << "Legal moves (" << currentLegalMoves.size() << "): ";
            for (const auto &move : currentLegalMoves)
            {
                std::cout << move.toString() << " ";
            }
            std::cout << "\n";
            continue;
        }

        // Try to parse as a move
        Move move = parseInput(input);
        if (!move.isValid())
        {
            std::cout << "Invalid input. Type 'help' for commands.\n";
            continue;
        }

        if (!tryMakeMove(move))
        {
            std::cout << "Illegal move. Try again.\n";
            continue;
        }

        printGameResult();
    }
}

// Input parsing - tries UCI first, then algebraic
Move Game::parseInput(const std::string &input) const
{
    // Try UCI format first (e.g., e2e4, e7e8q for promotion)
    Move m = parseUCI(input);
    if (m.isValid())
        return m;

    // Try algebraic notation (e.g., Nf3, 0-0)
    m = parseAlgebraic(input);
    return m;
}

Move Game::parseUCI(const std::string &input) const
{
    if (input.length() < 4 || input.length() > 5)
        return Move(); // Invalid length

    Square from = Square::fromAlgebraic(input.substr(0, 2));
    Square to = Square::fromAlgebraic(input.substr(2, 2));
    if (!from.isValid() || !to.isValid())
        return Move(); // Invalid squares

    PieceType promo = PieceType::NONE;
    if (input.length() == 5)
    {
        char p = std::tolower(input[4]);
        switch (p)
        {
        case 'q':
            promo = PieceType::QUEEN;
            break;
        case 'r':
            promo = PieceType::ROOK;
            break;
        case 'b':
            promo = PieceType::BISHOP;
            break;
        case 'n':
            promo = PieceType::KNIGHT;
            break;
        default:
            return Move(); // Invalid promotion piece
        }
    }

    // Match against legal moves to ensure it's valid
    for (auto &legal : currentLegalMoves)
    {
        if (legal.from == from && legal.to == to &&
            legal.promotionPiece == promo)
        {
            return legal; // Return the full move with flags set
        }
    }
    return Move(); // Not found in legal moves
}

static void stripAnnotations(std::string &s)
{
    while (!s.empty() && (s.back() == '+' || s.back() == '#' ||
                          s.back() == '!' || s.back() == '?'))
        s.pop_back();
}

static PieceType parsePiecePrefix(const std::string &s, int &outIdx)
{
    outIdx = 0;
    if (s.empty() || !std::isupper(s[0])) return PieceType::PAWN;
    switch (s[0])
    {
    case 'N': outIdx = 1; return PieceType::KNIGHT;
    case 'B': outIdx = 1; return PieceType::BISHOP;
    case 'R': outIdx = 1; return PieceType::ROOK;
    case 'Q': outIdx = 1; return PieceType::QUEEN;
    case 'K': outIdx = 1; return PieceType::KING;
    default:  return PieceType::NONE; // uppercase but not a piece letter
    }
}

static PieceType parsePromotionSuffix(std::string &s)
{
    if (s.length() < 2 || s[s.length() - 2] != '=') return PieceType::NONE;
    PieceType promo = PieceType::NONE;
    switch (s.back())
    {
    case 'Q': promo = PieceType::QUEEN;  break;
    case 'R': promo = PieceType::ROOK;   break;
    case 'B': promo = PieceType::BISHOP; break;
    case 'N': promo = PieceType::KNIGHT; break;
    default:  return PieceType::NONE;
    }
    s = s.substr(0, s.length() - 2);
    return promo;
}

static void parseDisambig(const std::string &disambig, int &file, int &rank)
{
    file = -1;
    rank = -1;
    for (char c : disambig)
    {
        if (c >= 'a' && c <= 'h') file = c - 'a';
        else if (c >= '1' && c <= '8') rank = c - '1';
    }
}

Move Game::parseAlgebraic(const std::string &input) const
{
    std::string s = input;
    stripAnnotations(s);
    if (s.empty()) return Move();

    // Castling
    if (s == "O-O" || s == "0-0")
    {
        for (auto &legal : currentLegalMoves)
            if (legal.isCastling && legal.to.col == 6) return legal;
        return Move();
    }
    if (s == "O-O-O" || s == "0-0-0")
    {
        for (auto &legal : currentLegalMoves)
            if (legal.isCastling && legal.to.col == 2) return legal;
        return Move();
    }

    // Piece prefix (N/B/R/Q/K) or pawn
    int startIdx = 0;
    PieceType type = parsePiecePrefix(s, startIdx);
    if (type == PieceType::NONE) return Move(); // Uppercase but unknown letter

    // Promotion suffix (=Q etc.)
    PieceType promo = parsePromotionSuffix(s);

    // Strip captures and piece prefix, leaving only destination + disambiguation
    std::string clean;
    for (char c : s.substr(startIdx))
        if (c != 'x') clean += c;

    if (clean.length() < 2) return Move();
    Square dest = Square::fromAlgebraic(clean.substr(clean.length() - 2));
    if (!dest.isValid()) return Move();

    int disambigFile, disambigRank;
    parseDisambig(clean.substr(0, clean.length() - 2), disambigFile, disambigRank);

    // Match against legal moves
    for (auto &legal : currentLegalMoves)
    {
        if (board.getPiece(legal.from).type != type) continue;
        if (legal.to != dest) continue;
        if (legal.promotionPiece != promo) continue;
        if (disambigFile >= 0 && legal.from.col != disambigFile) continue;
        if (disambigRank >= 0 && legal.from.row != disambigRank) continue;
        return legal;
    }
    return Move();
}

void Game::setMode(GameMode m)
{
    mode = m;
}

void Game::setDifficulty(Difficulty diff)
{
    ai.setDifficulty(diff);
}

void Game::setCPUColor(Color color)
{
    cpuColor = color;
}

void Game::makeCPUMove()
{
    std::cout << "CPU is thinking...\n";
    Move bestMove = ai.findBestMove(board);
    if (bestMove.isValid())
    {
        tryMakeMove(bestMove);
        std::cout << "CPU plays: " << bestMove.toString() << "\n";
    }
}

bool Game::isCPUTurn() const {
    if (result != GameResult::IN_PROGRESS) return false; // Game over
    if (mode != GameMode::HUMAN_VS_CPU) return false; // Not CPU mode
    return board.state.sideToMove == cpuColor;
}

std::string Game::getLastMoveString() const {
    if (moveHistory.empty()) return "";
    return moveHistory.back().toString();
}

std::optional<Move> Game::getLastMove() const {
    if (moveHistory.empty()) return std::nullopt;
    return moveHistory.back();
}




void Game::showMenu(GameMode &mode, Difficulty &diff, Color &cpuColor)
{
    std::cout << "Welcome to Chess!\n\n";
    std::cout << "Select Game Mode:\n";
    std::cout << "1. Human vs Human\n";
    std::cout << "2. Human vs CPU\n";
    std::cout << "\nEnter choice: ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "2")
    {
        mode = GameMode::HUMAN_VS_CPU;

        std::cout << "\nPlay as:\n";
        std::cout << "1. White (CPU plays Black)\n";
        std::cout << "2. Black (CPU plays White)\n";
        std::cout << "\nEnter choice: ";
        std::getline(std::cin, input);
        cpuColor = (input == "2") ? Color::WHITE : Color::BLACK;

        std::cout << "\nSelect Difficulty:\n";
        std::cout << "1. Easy\n";
        std::cout << "2. Medium\n";
        std::cout << "3. Hard\n";
        std::cout << "\nEnter choice: ";
        std::getline(std::cin, input);
        if (input == "1")
            diff = Difficulty::EASY;
        else if (input == "3")
            diff = Difficulty::HARD;
        else
            diff = Difficulty::MEDIUM;
    }
    else
    {
        mode = GameMode::HUMAN_VS_HUMAN;
    }

    std::cout << "\n";
}

void Game::updateScore() {
    if (result == GameResult::WHITE_WINS) whiteWins++;
    else if (result == GameResult::BLACK_WINS) blackWins++;
    else if (result != GameResult::IN_PROGRESS) drawCount++;
}

void Game::handleCPUTurn() {
    if (result != GameResult::IN_PROGRESS) return; // Game already over
    if (mode != GameMode::HUMAN_VS_CPU) return; // Not CPU mode
    if (board.state.sideToMove != cpuColor) return; // Not CPU's turn

    makeCPUMove();
}