#include "MoveGenerator.h"

// --- Pseudo-legal generation: scan all friendly pieces ---

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(Board& board) const {
    std::vector<Move> moves;
    moves.reserve(64);
    Color side = board.state.sideToMove; 
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

    for (auto& off : knightMoves) {
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
void MoveGenerator::generateCastlingMoves(const Board& board, Color color,
                                            std::vector<Move>& moves) const {
    // Cannot castle out of check
    if (board.isInCheck(color)) return;

    int row = (color == Color::WHITE) ? 0 : 7;
    Color enemy = oppositeColor(color);

    bool canKingside = (color == Color::WHITE)
        ? board.state.whiteCanCastleKingsside
        : board.state.blackCanCastleKingsside;
    
    bool canQueenside = (color == Color::WHITE)
        ? board.state.whiteCanCastleQueensside
        : board.state.blackCanCastleQueensside;

    // Kingside: king e -> g, square f and g must be empty and not attacked
    if (canKingside) {
        if (board.getPiece(row, 5).isEmpty() &&
            board.getPiece(row, 6).isEmpty() &&
            !board.isSquareAttacked(Square(row, 5), enemy) &&
            !board.isSquareAttacked(Square(row, 6), enemy)) {
            Move m(Square(row, 4), Square(row, 6));
            m.isCastling = true;
            moves.push_back(m);
        }
    }

    if (canQueenside) {
        if (board.getPiece(row, 1).isEmpty() &&
            board.getPiece(row, 2).isEmpty() &&
            board.getPiece(row, 3).isEmpty() &&
            !board.isSquareAttacked(Square(row, 2), enemy) &&
            !board.isSquareAttacked(Square(row, 3), enemy)) {
            Move m(Square(row, 4), Square(row, 2));
            m.isCastling = true;
            moves.push_back(m);
        }
    }
}

// --- Game ending queries ---
bool MoveGenerator::isCheckmate(Board& board) const {
    return board.isInCheck(board.state.sideToMove) &&
            generateLegalMoves(board).empty();
}

bool MoveGenerator::isStalemate(Board& board) const{
    return !board.isInCheck(board.state.sideToMove) &&
            generateLegalMoves(board).empty();   
}

bool MoveGenerator::isDraw(const Board& board) const {
    //50 move rule
    if (board.state.halfmoveClock >= 100) return true;

    //Insufficient material: count all pieces
    int whiteKnights = 0, whiteBishops = 0, whiteOther = 0;
    int blackKnights = 0, blackBishops = 0, blackOther = 0;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece p = board.getPiece(r, c);
            if (p.isEmpty() || p.type == PieceType::KING) continue;
            if (p.color == Color::WHITE) {
                if (p.type == PieceType::KNIGHT) whiteKnights++;
                else if (p.type == PieceType::BISHOP) whiteBishops++;
                else whiteOther++;
            } else {
                if (p.type == PieceType::KNIGHT) blackKnights++;
                else if (p.type == PieceType::BISHOP) blackBishops++;
                else blackOther++;
            }
        }
    }

    //If either side has, pawns, rooks, or queens, not sufficient
    if (whiteOther > 0 || blackOther > 0) return false;

    int whiteMinor = whiteKnights + whiteBishops;
    int blackMinor = blackKnights + blackBishops;

    // K vs K
    if (whiteMinor == 0 && blackMinor == 0) return true;
    // K+minor vs K
    if (whiteMinor <= 1 && blackMinor == 0) return true;
    if (blackMinor <= 1 && whiteMinor == 0) return true;

    return false;
}