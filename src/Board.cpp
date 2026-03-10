#include "Board.h"
#include "Move.h"
#include <iostream>

Board::Board() {
    reset();
}

void Board::reset() {
    //Clear board
    for (int row =0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            squares[row][col] = Piece();
        }
    }

    //Set up white pices (row 0 and 1
    squares[0][0] = Piece(PieceType::ROOK, Color::WHITE);
    squares[0][1] = Piece(PieceType::KNIGHT, Color::WHITE);
    squares[0][2] = Piece(PieceType::BISHOP, Color::WHITE);
    squares[0][3] = Piece(PieceType::QUEEN, Color::WHITE);
    squares[0][4] = Piece(PieceType::KING, Color::WHITE);
    squares[0][5] = Piece(PieceType::BISHOP, Color::WHITE);
    squares[0][6] = Piece(PieceType::KNIGHT, Color::WHITE);
    squares[0][7] = Piece(PieceType::ROOK, Color::WHITE);

    for (int col = 0; col < 8; ++col) {
        squares[1][col] = Piece(PieceType::PAWN, Color::WHITE);
    }

    //Set up black pieces (row 7 and 6)
    squares[7][0] = Piece(PieceType::ROOK, Color::BLACK);
    squares[7][1] = Piece(PieceType::KNIGHT, Color::BLACK);
    squares[7][2] = Piece(PieceType::BISHOP, Color::BLACK);
    squares[7][3] = Piece(PieceType::QUEEN, Color::BLACK);
    squares[7][4] = Piece(PieceType::KING, Color::BLACK);
    squares[7][5] = Piece(PieceType::BISHOP, Color::BLACK);
    squares[7][6] = Piece(PieceType::KNIGHT, Color::BLACK);
    squares[7][7] = Piece(PieceType::ROOK, Color::BLACK);

    for (int col = 0; col < 8; ++col) {
        squares[6][col] = Piece(PieceType::PAWN, Color::BLACK);
    }
}

Piece Board::getPiece(const Square& sq) const {
    if (!sq.isValid()) return Piece();
    return squares[sq.row][sq.col];
}

Piece Board::getPiece(int row, int col) const {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return Piece();
    return squares[row][col];
}

void Board::setPiece(const Square& sq, const Piece& piece) {
    if (sq.isValid()) {
        squares[sq.row][sq.col] = piece;
    }
}

void Board::setPiece(int row, int col, const Piece& piece) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        squares[row][col] = piece;
    }
}

void Board::print() const {
    std::cout << "\n";
    for (int row = 7; row >= 0; --row) {
        std::cout << (row + 1) << " ";
        for (int col = 0; col < 8; ++col) {
            std::cout << pieceToChar(squares[row][col]) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

Square Board::findKing(Color color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece& p = squares[row][col];
            if (p.type == PieceType::KING && p.color == color) {
                return Square(row, col);
            }
        }
    }
    return Square(); // Invalid square if not found
}

bool Board::isSquareAttacked(const Square& sq, Color byColor) const {
    //Check knight attacks
    const int knightOffsets[8][2] = {
        {-2,-1}, {-2,1}, {-1,-2}, {-1,2},
        {1,-2}, {1,2}, {2,-1}, {2,1}
    };
    for (auto& off : knightOffsets) {
        int r = sq.row + off[0], c = sq.col + off[1];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            Piece p = squares[r][c];
            if (p.type == PieceType::KNIGHT && p.color == byColor)
                return true;
        }
    }

    //check pawn attacks
    int pawnDir = (byColor == Color::WHITE) ? 1 : -1;
    // Pawns of bycolor attack FROM pawnDir rows away
    //If byColor is WHITE, white pawns sit below and attack upwards
    //If byColor is BLACK, black pawns sit above and attack downwards
    int pawnRow = sq.row - pawnDir; // Pawns attack from the opposite direction
    for (int dc : {-1, 1}) {
        int c = sq.col + dc;
        if (pawnRow >= 0 && pawnRow < 8 && c >= 0 && c < 8) {
            Piece p = squares[pawnRow][c];
            if (p.type == PieceType::PAWN && p.color == byColor)
            return true;
        }
    }

    //Check king attacks (adj sq)
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++ dc) {
            if (dr == 0 && dc == 0) continue;
            int r = sq.row + dr, c = sq.col + dc;
            if (r >= 0 && r < 8 && c >= 0 && c < 8) {
                Piece p = squares[r][c];
                if (p.type == PieceType::KING && p.color == byColor)
                    return true;
            }
        }
    }

    //Check sliding piece attacks (rook/bishop/queen)
    //Diag directions for bishop/queen
    const int diagDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto& dir : diagDirs) {
        for (int dist = 1; dist < 8; ++dist) {
            int r = sq.row + dir[0] * dist;
            int c = sq.col + dir[1] * dist;
            if (r < 0 || r >= 8 || c < 0 || c>= 8) break;
            Piece p = squares[r][c];
            if (!p.isEmpty()) {
                if (p.color == byColor && 
                    (p.type == PieceType::BISHOP || p.type == PieceType::QUEEN))
                    return true;
                break; //Blocked by a piece
            }
        }
    }

    //Orthogonal directions for rook/queen
    const int orthoDir[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto& dir : orthoDir) {
        for (int dist = 1; dist < 8; ++dist) {
            int r = sq.row + dir[0] * dist;
            int c = sq.col + dir[1] * dist;
            if (r < 0 || r >= 8 || c < 0 || c >= 8) break;
            Piece p = squares[r][c];
            if (!p.isEmpty()) {
                if (p.color == byColor &&
                    (p.type == PieceType::ROOK || p.type == PieceType::QUEEN))
                    return true;
                break; //Blocked by a piece
            }
        }
    }
    return false;
}

bool Board::isInCheck( Color color) const {
    Square king = findKing(color);
    return king.isValid() && isSquareAttacked(king, oppositeColor(color));
}


void Board :: makeMove(Move& move) {
    //Save current state for undo
    stateHistory.push_back(state);

    Piece movingPiece = getPiece(move.from);
    Piece targetPiece = getPiece(move.to);

    //Record captured piece (for normal captures)
    move.capturedPiece = targetPiece;

    // Handle en passant capture
    if (move.isEnPassant) {
        //Captured pawn is on the same row as "from", same col as "to"
        move.capturedPiece = getPiece(move.from.row, move.to.col);
        setPiece(move.from.row, move.to.col, Piece()); //Remove captured pawn
    }

    //Move the piece
    setPiece(move.to, movingPiece);
    setPiece(move.from, Piece());

    //Handle castling - move the rook
    if (move.isCastling) {
        int row = move.from.row;
        if (move.to.col == 6) { //Kingside
            setPiece(row, 5, getPiece(row, 7)); //rook h -> f
            setPiece(row, 7, Piece());
        } else if (move.to.col == 2) { //Queenside
            setPiece(row, 3, getPiece(row, 0)); // rook a -> d
            setPiece(row, 0, Piece());
        }
    }

    //Handle promotion
    if (move.isPromotion()) {
        setPiece(move.to, Piece(move.promotionPiece, movingPiece.color));
    }

    //Update castling rights
    //King moves - revoke both sides
    if (movingPiece.type == PieceType::KING) {
        if (movingPiece.color == Color::WHITE) {
            state.whiteCanCastleKingsside = false;
            state.whiteCanCastleQueensside = false;
        } else {
            state.blackCanCastleKingsside = false;
            state.blackCanCastleQueensside = false;
        }
    }

    //Rook move or is captured - revoke relevant side
    if (move.from == Square(0, 0) || move.to == Square(0, 0))
        state.whiteCanCastleQueensside = false;
    if (move.from == Square(0, 7) || move.to == Square(0, 7))
        state.whiteCanCastleKingsside = false;
    if (move.from == Square(7, 0) || move.to == Square(7, 0))
        state.blackCanCastleQueensside = false;
    if (move.from == Square(7, 7) || move.to == Square(7, 7))
        state.blackCanCastleKingsside = false;

    //Update en passant target
    if (move.isDoublePawnPush) {
        state.enPassantTarget = Square((move.from.row + move.to.row) / 2, move.from.col);
    } else {
        state.enPassantTarget = Square(); //Invalid square = no en passant
    }

    //Update halfmove clock
    if (movingPiece.type == PieceType::PAWN || move.isCapture || move.isEnPassant) {
        state.halfmoveClock = 0;
    } else {
        state.halfmoveClock++;
    }

    //Update fullmove number
    if (state.sideToMove == Color::BLACK) {
        state.fullmoveNumber++;
    }

    //Toggle side to move
    state.sideToMove = oppositeColor(state.sideToMove);
}

void Board::undoMove(const Move& move) {
    //Restore state
    state = stateHistory.back();
    stateHistory.pop_back();

    Piece movingPiece = getPiece(move.to);

    //Reverse promotion - change back to pawn
    if (move.isPromotion()) {
        movingPiece = Piece(PieceType::PAWN, movingPiece.color);
    }

    //Move piece back
    setPiece(move.from, movingPiece);
    setPiece(move.to, Piece());

    //Restore captured piece
    if (move.isEnPassant) {
        //Put captured pawn back on the square it was captured from (same row as "from", same col as "to")
        setPiece(move.from.row, move.to.col, move.capturedPiece);
    } else if (move.isCapture) {
        setPiece(move.to, move.capturedPiece);
    }
    
    //Reverse castiling - move the rook back
    if (move.isCastling) {
        int row = move.from.row;    
        if (move.to.col == 6) { //Kingside
            setPiece(row, 7, getPiece(row, 5)); //rook f-> h
            setPiece(row, 5, Piece());
        } else if (move.to.col == 2) { //Queenside
            setPiece(row, 0, getPiece(row, 3)); //rook d -> a
            setPiece(row, 3, Piece());
        }
    }
}