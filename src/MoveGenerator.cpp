#include "MoveGenerator.h"

// --- Pseudo-legal generation: scan all friendly pieces ---

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    moves.reserve(64);
    Color side = board.state.sideToMove; {
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                Piece p = board.getPiece(row, col);
                if (p.isEmpty() || p.color != side) continue;
                
                Square from(row, col);
            switch (p.type) {
                case PieceType::PAWN:
                    generatePawnMoves(board, from, side, moves);
                    break;
                case PieceType::KNIGHT:
                    generateKnightMoves(board, from, side, moves);
                    break;
                case PieceType::BISHOP: {
                    const int dirs[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
                    generateSlidingMoves(board, from, side, dirs, 4, moves);
                    break;
                }
                case PieceType::ROOK: {
                    const int dirs[4][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
                    generateSlidingMoves(board, from, side, dirs, 4, moves);
                    break;
                }
                case PieceType::QUEEN: {
                    const int dirs[8][2] = {{1,0}, {-1,0}, {0,1}, {0,-1},
                                            {1,1}, {1,-1}, {-1,1}, {-1,-1}};
                    generateSlidingMoves(board, from, side, dirs, 8, moves);
                    break;
                }
                case PieceType::KING:
                    generateKingMoves(board, from, side, moves);
                    break;
                default:
                    break;
            }
        }
    }
    generateCastlingMoves(board, side, moves);
    return moves;
}

// --- Legal generation: make/unmake and check if king is safe ---

std::vector<Move> MoveGenerator::generateLegalMoves(Board& board) const {
    std::vector<Move> pseudoLegal = generatePseudoLegalMoves(board);
    std::vector<Move> legal;
    legal.reserve(pseudoLegal.size());

    Color side = board.state.sideToMove;
    for (auto& move : pseudoLegal) {
        board.makeMove(move);
        if (!board.isInCheck(side)) {
            legal.push_back(move);
        }
        board.undoMove(move);
    }
    return legal;
}

// --- Pawn moves ---

void MoveGenerator::addPawnMove(const Square& from, const Square& to, Color color,
                                bool isCapture, std::vector<Move>& moves) const {
    int promoRank = (color == Color::WHITE) ? 7 : 0;

    if (to.row == promoRank) {
        //Generate 4 promotion moves
        PieceType promos[] = {PieceType::QUEEN, PieceType::ROOK, PieceType::BISHOP, PieceType::KNIGHT};

        for (PieceType pt : promos) {
            Move m(from, to);
            m.isCapture = isCapture;
            m.promotionPiece = pt;
            moves.push_back(m);
        }
    } else {
        Move m(from, to);
        m.isCapture = isCapture;
        moves.push_back(m);
    }
}

void MoveGenerator::generatePawnMoves(const Board& board, const Square& from, 
                                    Color color, std::vector<Move>& moves) const {
    int dir = (color == Color::WHITE) ? 1 : -1;
    int startRow = (color == Color::WHITE) ? 1 : 6;


    //Single push
    Square oneStep(from.row + dir, from.col);
    if (oneStep.isValid() && board.getPiece(oneStep).isEmpty()) {
        addPawnMove(from, oneStep, color, false, moves);

        //Double push
        Square twoStep(from.row + 2 * dir, from.col);
        if (from.row == startRow && board.getPiece(twoStep).isEmpty()) {
            Move m(from, twoStep);
            m.isDoublePawnPush = true;
            moves.push_back(m);
        }
    }

    //Captures (including en passant)
    for (int dc : {-1, 1}) {
        Square target(from.row + dir, from.col + dc);
        if (!target.isValid()) continue;

        Piece targetPiece = board.getPiece(target);
        if (!targetPiece.isEmpty() && targetPiece.color != color) {
            addPawnMove(from, target, color, true, moves);
        }

        //En passants
        if (target == board.state.enPassantTarget) {
            Move m(from, target);
            m.isCapture = true;
            m.isEnPassant = true;
            moves.push_back(m);
        }
    }
}

// --- Knight moves ---
void MoveGenerator::generateKnightMoves(const Board & board, const Square& from, Color color, std::vector<Move>& moves) const {
    
    const int knightMoves[8][2] = {
        {-2,-1}, {-2,1}, {-1,-2}, {-1,2},
        {1,-2}, {1,2}, {2,-1}, {2,1}
    };

    for (auto& off : offsets) {
        Square to(from.row + off[0], from.col + off[1]);
        if (!to.isValid()) continue;

        Piece target = board.getPiece(to);
        if (target.isEmpty()) {
            moves.push_back(Move(from, to));
        } else if (target.color != color) {
            Move m(from, to);
            m.isCapture = true;
            moves.push_back(m);
        }
    }
}

// --- Sliding pieces (Bishops, Rooks, Queens) ---
void MoveGenerator::generateSlidingMoves(const Board& board, const Square& from, 
                                            Color color, const int directions[][2], 
                                            int numDirections, 
                                            std::vector<Move>& moves) const {
    for (int d = 0; d < numDirections; ++d) {
        for (int dist = 1; dist < 8; ++dist) {
            Square to(from.row + directions[d][0] * dist,
                        from.col + directions[d][1] * dist);
            if (!to.isValid()) break;

            Piece target = board.getPiece(to);
            if (target.isEmpty()) {
                moves.push_back(Move(from, to));    
            } else {
                if (target.color != color) {
                    Move m(from, to);
                    m.isCapture = true;
                    moves.push_back(m);
                }
                break; //Blocked
            }
        }
    }
}

// --- King moves (excluding castling) ---
void MoveGenerator::generateKingMoves(const Board& board, const Square& from,
                                        Color color, std::vector<Move>& moves) const {
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            Square to(from.row + dr, from.col + dc);
            if (!to.isValid()) continue;

            Piece target = board.getPiece(to);
            if (target.isEmpty()) {
                moves.push_back(Move(from, to));
            } else if (target.color != color) {
                Move m(from, to);
                m.isCapture = true;
                moves.push_back(m);
            }
        }
    }
}

// --- Castling moves ---
