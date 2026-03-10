#include "AI.h"
#include <algorithm>
#include <climits>

// Piece-square tables for positional evaluation
// Values in centipawns - bonuses for good piece placement

static const int PAWN_TABLE[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {50, 50, 50, 50, 50, 50, 50, 50},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

static const int KNIGHT_TABLE[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int BISHOP_TABLE[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int ROOK_TABLE[8][8] = {
    { 0,  0,  0,  5,  5,  0,  0,  0},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

static const int KING_TABLE[8][8] = {
    { 20, 30, 10,  0,  0, 10, 30, 20},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30}
};

AI::AI(Difficulty diff) : difficulty(diff) {}

void AI::setDifficulty(Difficulty diff) {
    difficulty = diff;
}

int AI::getSearchDepth() const {
    switch (difficulty) {
        case Difficulty::EASY: return 2;
        case Difficulty::MEDIUM: return 4;
        case Difficulty::HARD: return 6;
        default: return 4;
    }
}

int AI::pieceValue(PieceType type) const {
    switch (type) {
        case PieceType::PAWN: return 100;
        case PieceType::KNIGHT: return 320;
        case PieceType::BISHOP: return 330;
        case PieceType::ROOK: return 500;
        case PieceType::QUEEN: return 900;
        case PieceType::KING: return 20000;
        default: return 0;
    }
}

int AI::positionBonus(PieceType type, Color color, int row, int col) const {
    //For blacck, mirror the table vertically
    int r = (color == Color::WHITE) ? row : (7 - row);
    
    switch (type) {
        case PieceType::PAWN: return PAWN_TABLE[r][col];
        case PieceType::KNIGHT: return KNIGHT_TABLE[r][col];
        case PieceType::BISHOP: return BISHOP_TABLE[r][col];
        case PieceType::ROOK: return ROOK_TABLE[r][col];
        case PieceType::KING: return KING_TABLE[r][col];
        default: return 0;
    }
}

int AI::evaluate(const Board& board) const {
    int score = 0;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Piece p = board.getPiece(row, col);
            if (p.isEmpty()) continue;

            int value = pieceValue(p.type) + positionBonus(p.type, p.color, row, col);

            if (p.color == Color::WHITE) {
                score += value;
            } else {
                score -= value;
            }
        }
    }
    return score;
}

int AI::minimax(Board& board, int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) {
        return evaluate(board);
    }

    std::vector<Move> moves = moveGen.generateLegalMoves(board);

    //Checkmate or stalemate
    if (moves.empty()) {
        if (board.isInCheck(board.state.sideToMove)) {
            //Checkmate - return large negative score for maximizing, positive for minimizing
            //Add depth so the AI prefers faster checkmates and slower losses
            return maximizing ? (-100000 - depth) : (100000 + depth);
        }
        return 0; //Stalemate
    }

    if (maximizing) {
        int maxEval = INT_MIN;
        for (auto& move : moves) {
            board.makeMove(move);
            int eval = minimax(board, depth - 1, alpha, beta, false);
            board.undoMove(move);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break; //Beta cut-off
        }
        return maxEval;
    }else {
        int minEval = INT_MAX;
        for (auto& move : moves) {
            board.makeMove(move);
            int eval = minimax(board, depth - 1, alpha, beta, true);
            board.undoMove(move);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break; //Alpha cut-off
        }
        return minEval;
    }
}

Move AI::findBestMove(Board& board) {
    std::vector<Move> moves = moveGen.generateLegalMoves(board);
    if (moves.empty()) return Move(); //No legal moves

    int depth = getSearchDepth();
    bool maximizing = (board.state.sideToMove == Color::WHITE);

    Move bestMove = moves[0];
    int bestEval = maximizing ? INT_MIN : INT_MAX;

    for (auto& move : moves) {
        board.makeMove(move);
        int eval = minimax(board, depth - 1, INT_MIN, INT_MAX, !maximizing);
        board.undoMove(move);

        if (maximizing) {
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }else {
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = move;
            }
        }
    }

    return bestMove;
}
