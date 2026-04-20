#include "AI.h"
#include <algorithm>
#include <climits>

// Piece-square tables for positional evaluation
// Values in centipawns - bonuses for good piece placement

static const int PAWN_TABLE[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {5, 10, 10, -20, -20, 10, 10, 5},
    {5, -5, -10, 0, 0, -10, -5, 5},
    {0, 0, 0, 20, 20, 0, 0, 0},
    {5, 5, 10, 25, 25, 10, 5, 5},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {0, 0, 0, 0, 0, 0, 0, 0}};

static const int KNIGHT_TABLE[8][8] = {
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20, 0, 5, 5, 0, -20, -40},
    {-30, 5, 10, 15, 15, 10, 5, -30},
    {-30, 0, 15, 20, 20, 15, 0, -30},
    {-30, 5, 15, 20, 20, 15, 5, -30},
    {-30, 0, 10, 15, 15, 10, 0, -30},
    {-40, -20, 0, 0, 0, 0, -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50}};

static const int BISHOP_TABLE[8][8] = {
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10, 5, 0, 0, 0, 0, 5, -10},
    {-10, 10, 10, 10, 10, 10, 10, -10},
    {-10, 0, 10, 10, 10, 10, 0, -10},
    {-10, 5, 5, 10, 10, 5, 5, -10},
    {-10, 0, 5, 10, 10, 5, 0, -10},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20}};

static const int ROOK_TABLE[8][8] = {
    {0, 0, 0, 5, 5, 0, 0, 0},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {5, 10, 10, 10, 10, 10, 10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0}};

static const int KING_TABLE[8][8] = {
    {20, 30, 10, 0, 0, 10, 30, 20},
    {20, 20, 0, 0, 0, 0, 20, 20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30}};

AI::AI(Difficulty diff) : difficulty(diff) { clearKillerMoves(); }

void AI::setDifficulty(Difficulty diff)
{
    difficulty = diff;
}

int AI::getSearchDepth() const
{
    switch (difficulty)
    {
    case Difficulty::EASY:
        return 2;
    case Difficulty::MEDIUM:
        return 4;
    case Difficulty::HARD:
        return 5;
    default:
        return 4;
    }
}

int AI::pieceValue(PieceType type) const
{
    switch (type)
    {
    case PieceType::PAWN:
        return 100;
    case PieceType::KNIGHT:
        return 320;
    case PieceType::BISHOP:
        return 330;
    case PieceType::ROOK:
        return 500;
    case PieceType::QUEEN:
        return 900;
    case PieceType::KING:
        return 20000;
    default:
        return 0;
    }
}

int AI::positionBonus(PieceType type, Color color, int row, int col) const
{
    // For black, mirror the table vertically
    int r = (color == Color::WHITE) ? row : (7 - row);

    switch (type)
    {
    case PieceType::PAWN:
        return PAWN_TABLE[r][col];
    case PieceType::KNIGHT:
        return KNIGHT_TABLE[r][col];
    case PieceType::BISHOP:
        return BISHOP_TABLE[r][col];
    case PieceType::ROOK:
        return ROOK_TABLE[r][col];
    case PieceType::KING:
        return KING_TABLE[r][col];
    default:
        return 0;
    }
}

int AI::scoreMove(const Move &move, const Board &board, int depth) const
{
    int score = 0;

    // Captures: prioritize capturing high-value pieces and low-value pieces
    if (move.isCapture)
    {
        int victimVal = pieceValue(move.capturedPiece.type);
        int attackerVal = pieceValue(board.getPiece(move.from).type);
        score += 10000 * victimVal - attackerVal; // Prioritize high victim value and low attacker value
    }

    // Promotions: are very valuable, especially to queen
    if (move.isPromotion())
    {
        score += 9000 + pieceValue(move.promotionPiece);
    }

    // Killer move heuristic - non-capture moves that caused cutoffs
    if (!move.isCapture && depth >= 0 && depth < MAX_DEPTH)
    {
        if (move == killerMoves[depth][0]) score += 9000;
        else if (move == killerMoves[depth][1]) score += 8000;
    }

    return score;
}

int AI::evaluate(const Board &board) const
{
    int score = 0;

    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            Piece p = board.getPiece(row, col);
            if (p.isEmpty())
                continue;

            int value = pieceValue(p.type) + positionBonus(p.type, p.color, row, col);

            if (p.color == Color::WHITE)
            {
                score += value;
            }
            else
            {
                score -= value;
            }
        }
    }

    score += evaluateKingSafety(board, Color::WHITE);
    score -= evaluateKingSafety(board, Color::BLACK);
    score += evaluatePawnStructure(board);
    score += evaluateBishopPair(board);

    return score;
}

int AI::minimax(Board &board, int depth, int alpha, int beta, bool maximizing)
{
    if (depth == 0)
    {
        return quiescence(board, alpha, beta, maximizing);
    }

    std::vector<Move> moves = moveGen.generateLegalMoves(board);

// Sort moves for better alpha-beta pruning (captures and promotions first)
    std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b)
         { return scoreMove(a, board, depth) > scoreMove(b, board, depth); });

    // Checkmate or stalemate
    if (moves.empty())
    {
        if (board.isInCheck(board.state.sideToMove))
        {
            // Checkmate - return large negative score for maximizing, positive for minimizing
            // Add depth so the AI prefers faster checkmates and slower losses
            return maximizing ? (-100000 - depth) : (100000 + depth);
        }
        return 0; // Stalemate
    }

    if (maximizing)
    {
        int maxEval = INT_MIN;
        for (auto &move : moves)
        {
            board.makeMove(move);
            int eval = minimax(board, depth - 1, alpha, beta, false);
            board.undoMove(move);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                storeKillerMove(move, depth);
                break; // Beta cut-off
            }
        }
        return maxEval;
    }
    else
    {
        int minEval = INT_MAX;
        for (auto &move : moves)
        {
            board.makeMove(move);
            int eval = minimax(board, depth - 1, alpha, beta, true);
            board.undoMove(move);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
            {
                storeKillerMove(move, depth);
                break; // Alpha cut-off
            }
        }
        return minEval;
    }
}

Move AI::findBestMove(Board &board)
{
    clearKillerMoves();
    std::vector<Move> moves = moveGen.generateLegalMoves(board);
    int depth = getSearchDepth();
    std::sort(moves.begin(), moves.end(), [&](const Move &a, const Move &b)
         { return scoreMove(a, board, depth) > scoreMove(b, board, depth); });

    if (moves.empty())
        return Move(); // No legal moves

    bool maximizing = (board.state.sideToMove == Color::WHITE);

    Move bestMove = moves[0];
    int bestEval = maximizing ? INT_MIN : INT_MAX;

    for (auto &move : moves)
    {
        board.makeMove(move);
        int eval = minimax(board, depth - 1, INT_MIN, INT_MAX, !maximizing);
        board.undoMove(move);

        if (maximizing)
        {
            if (eval > bestEval)
            {
                bestEval = eval;
                bestMove = move;
            }
        }
        else
        {
            if (eval < bestEval)
            {
                bestEval = eval;
                bestMove = move;
            }
        }
    }

    return bestMove;
}

// --- Quiescence Search ---

int AI::quiescence(Board& board, int alpha, int beta, bool maximizing)
{
    int standPat = evaluate(board);

    if (maximizing)
    {
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;

        std::vector<Move> captures = moveGen.generateLegalCaptures(board);
        std::sort(captures.begin(), captures.end(),
            [&](const Move& a, const Move& b) {
                return scoreMove(a, board) > scoreMove(b, board);
            });

        for (auto& move : captures)
        {
            board.makeMove(move);
            int eval = quiescence(board, alpha, beta, false);
            board.undoMove(move);
            if (eval > alpha) alpha = eval;
            if (alpha >= beta) break;
        }
        return alpha;
    }
    else
    {
        if (standPat <= alpha) return alpha;
        if (standPat < beta) beta = standPat;

        std::vector<Move> captures = moveGen.generateLegalCaptures(board);
        std::sort(captures.begin(), captures.end(),
            [&](const Move& a, const Move& b) {
                return scoreMove(a, board) > scoreMove(b, board);
            });

        for (auto& move : captures)
        {
            board.makeMove(move);
            int eval = quiescence(board, alpha, beta, true);
            board.undoMove(move);
            if (eval < beta) beta = eval;
            if (alpha >= beta) break;
        }
        return beta;
    }
}

// --- King Safety Evaluation ---

int AI::evaluateKingSafety(const Board& board, Color color) const
{
    int safety = 0;
    Square king = board.findKing(color);
    if (!king.isValid()) return 0;

    Color enemy = oppositeColor(color);
    int homeRow = (color == Color::WHITE) ? 0 : 7;

    // Penalize king stuck in center after move 10
    bool castled = (king.row == homeRow && (king.col <= 2 || king.col >= 6));
    if (!castled && board.state.fullmoveNumber > 10)
    {
        if (king.col >= 3 && king.col <= 4)
            safety -= 30;
    }

    // Pawn shield bonus
    int pawnDir = (color == Color::WHITE) ? 1 : -1;
    for (int dc = -1; dc <= 1; ++dc)
    {
        int r = king.row + pawnDir;
        int c = king.col + dc;
        if (r >= 0 && r < 8 && c >= 0 && c < 8)
        {
            Piece p = board.getPiece(r, c);
            if (p.type == PieceType::PAWN && p.color == color)
                safety += 10;
            else
                safety -= 10;
        }
    }

    // Penalize attacked squares near king
    int attackerCount = 0;
    for (int dr = -1; dr <= 1; ++dr)
    {
        for (int dc = -1; dc <= 1; ++dc)
        {
            Square sq(king.row + dr, king.col + dc);
            if (sq.isValid() && board.isSquareAttacked(sq, enemy))
                attackerCount++;
        }
    }
    safety -= attackerCount * 8;

    return safety;
}

// --- Pawn Structure Evaluation ---

int AI::evaluatePawnStructure(const Board& board) const
{
    int score = 0;
    int whitePawns[8] = {0};
    int blackPawns[8] = {0};

    for (int col = 0; col < 8; ++col)
        for (int row = 0; row < 8; ++row)
        {
            Piece p = board.getPiece(row, col);
            if (p.type == PieceType::PAWN)
            {
                if (p.color == Color::WHITE) whitePawns[col]++;
                else blackPawns[col]++;
            }
        }

    for (int col = 0; col < 8; ++col)
    {
        // Doubled pawns
        if (whitePawns[col] > 1) score -= 15 * (whitePawns[col] - 1);
        if (blackPawns[col] > 1) score += 15 * (blackPawns[col] - 1);

        // Isolated pawns
        bool whiteIso = whitePawns[col] > 0 &&
            (col == 0 || whitePawns[col-1] == 0) &&
            (col == 7 || whitePawns[col+1] == 0);
        bool blackIso = blackPawns[col] > 0 &&
            (col == 0 || blackPawns[col-1] == 0) &&
            (col == 7 || blackPawns[col+1] == 0);
        if (whiteIso) score -= 15;
        if (blackIso) score += 15;
    }

    // Passed pawns
    for (int col = 0; col < 8; ++col)
        for (int row = 0; row < 8; ++row)
        {
            Piece p = board.getPiece(row, col);
            if (p.type != PieceType::PAWN) continue;

            bool passed = true;
            if (p.color == Color::WHITE)
            {
                for (int r = row + 1; r < 8 && passed; ++r)
                    for (int dc = -1; dc <= 1; ++dc)
                    {
                        int c = col + dc;
                        if (c < 0 || c >= 8) continue;
                        Piece b = board.getPiece(r, c);
                        if (b.type == PieceType::PAWN && b.color == Color::BLACK)
                            passed = false;
                    }
                if (passed) score += 20 + row * 5;
            }
            else
            {
                for (int r = row - 1; r >= 0 && passed; --r)
                    for (int dc = -1; dc <= 1; ++dc)
                    {
                        int c = col + dc;
                        if (c < 0 || c >= 8) continue;
                        Piece b = board.getPiece(r, c);
                        if (b.type == PieceType::PAWN && b.color == Color::WHITE)
                            passed = false;
                    }
                if (passed) score -= 20 + (7 - row) * 5;
            }
        }

    return score;
}

// --- Bishop Pair Evaluation ---

int AI::evaluateBishopPair(const Board& board) const
{
    int wb = 0, bb = 0;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
        {
            Piece p = board.getPiece(r, c);
            if (p.type == PieceType::BISHOP)
            {
                if (p.color == Color::WHITE) wb++;
                else bb++;
            }
        }
    int score = 0;
    if (wb >= 2) score += 30;
    if (bb >= 2) score -= 30;
    return score;
}

// --- Killer Move Heuristic ---

bool AI::isKillerMove(const Move& move, int depth) const
{
    if (depth < 0 || depth >= MAX_DEPTH) return false;
    return (move == killerMoves[depth][0]) || (move == killerMoves[depth][1]);
}

void AI::storeKillerMove(const Move& move, int depth)
{
    if (depth < 0 || depth >= MAX_DEPTH) return;
    if (move.isCapture) return; // Only store quiet moves
    if (!(move == killerMoves[depth][0]))
    {
        killerMoves[depth][1] = killerMoves[depth][0];
        killerMoves[depth][0] = move;
    }
}

void AI::clearKillerMoves()
{
    for (int i = 0; i < MAX_DEPTH; ++i)
    {
        killerMoves[i][0] = Move();
        killerMoves[i][1] = Move();
    }
}
