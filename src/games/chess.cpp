/**
 * Chess Game Implementation
 */

#include "games/chess.h"
#include <stdlib.h>

void Chess::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    currentPlayer = PieceColor::WHITE;
    playerIsWhite = true;
    pieceSelected = false;
    selectedX = selectedY = -1;
    cursorX = cursorY = 0;
    inCheck = false;
    checkmate = false;
    stalemate = false;
    enPassantX = enPassantY = -1;

    memset(validMoves, 0, sizeof(validMoves));
    setupBoard();
}

void Chess::setupBoard() {
    // Clear board
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            board[x][y] = {Piece::EMPTY, PieceColor::NONE, false};
        }
    }

    // Setup pawns
    for (int x = 0; x < 8; x++) {
        board[x][1] = {Piece::PAWN, PieceColor::BLACK, false};
        board[x][6] = {Piece::PAWN, PieceColor::WHITE, false};
    }

    // Setup black pieces (top)
    board[0][0] = {Piece::ROOK, PieceColor::BLACK, false};
    board[1][0] = {Piece::KNIGHT, PieceColor::BLACK, false};
    board[2][0] = {Piece::BISHOP, PieceColor::BLACK, false};
    board[3][0] = {Piece::QUEEN, PieceColor::BLACK, false};
    board[4][0] = {Piece::KING, PieceColor::BLACK, false};
    board[5][0] = {Piece::BISHOP, PieceColor::BLACK, false};
    board[6][0] = {Piece::KNIGHT, PieceColor::BLACK, false};
    board[7][0] = {Piece::ROOK, PieceColor::BLACK, false};

    // Setup white pieces (bottom)
    board[0][7] = {Piece::ROOK, PieceColor::WHITE, false};
    board[1][7] = {Piece::KNIGHT, PieceColor::WHITE, false};
    board[2][7] = {Piece::BISHOP, PieceColor::WHITE, false};
    board[3][7] = {Piece::QUEEN, PieceColor::WHITE, false};
    board[4][7] = {Piece::KING, PieceColor::WHITE, false};
    board[5][7] = {Piece::BISHOP, PieceColor::WHITE, false};
    board[6][7] = {Piece::KNIGHT, PieceColor::WHITE, false};
    board[7][7] = {Piece::ROOK, PieceColor::WHITE, false};
}

bool Chess::update() {
    if (checkmate || stalemate) {
        gameOver = true;
    }

    // AI's turn
    if (!gameOver && currentPlayer != (playerIsWhite ? PieceColor::WHITE : PieceColor::BLACK)) {
        Move aiMove = findBestMove(currentPlayer, 3);
        if (aiMove.isValid) {
            makeMove(aiMove.fromX, aiMove.fromY, aiMove.toX, aiMove.toY);
            lastMove = aiMove;

            // Switch player
            currentPlayer = (currentPlayer == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

            // Check game state
            inCheck = isKingInCheck(currentPlayer);
            if (!hasLegalMoves(currentPlayer)) {
                if (inCheck) {
                    checkmate = true;
                } else {
                    stalemate = true;
                }
            }
            needsRedraw = true;
        }
    }

    return !exitRequested;
}

void Chess::handleInput(Button btn) {
    if (gameOver) {
        if (btn == Button::CONFIRM) {
            init();
            needsRedraw = true;
        } else if (btn == Button::BACK) {
            exitRequested = true;
        }
        return;
    }

    // Only allow input on player's turn
    if (currentPlayer != (playerIsWhite ? PieceColor::WHITE : PieceColor::BLACK)) {
        return;
    }

    switch (btn) {
        case Button::LEFT:
            if (cursorX > 0) cursorX--;
            needsRedraw = true;
            break;

        case Button::RIGHT:
            if (cursorX < 7) cursorX++;
            needsRedraw = true;
            break;

        case Button::UP:
            if (cursorY > 0) cursorY--;
            needsRedraw = true;
            break;

        case Button::DOWN:
            if (cursorY < 7) cursorY++;
            needsRedraw = true;
            break;

        case Button::CONFIRM:
            if (!pieceSelected) {
                // Try to select a piece
                if (board[cursorX][cursorY].color == currentPlayer) {
                    selectedX = cursorX;
                    selectedY = cursorY;
                    pieceSelected = true;
                    calculateValidMoves(cursorX, cursorY);
                }
            } else {
                // Try to move the selected piece
                if (validMoves[cursorX][cursorY]) {
                    makeMove(selectedX, selectedY, cursorX, cursorY);
                    lastMove = {selectedX, selectedY, cursorX, cursorY, Piece::EMPTY, true, 0};

                    // Handle pawn promotion
                    if (board[cursorX][cursorY].piece == Piece::PAWN) {
                        if ((board[cursorX][cursorY].color == PieceColor::WHITE && cursorY == 0) ||
                            (board[cursorX][cursorY].color == PieceColor::BLACK && cursorY == 7)) {
                            handlePromotion(cursorX, cursorY);
                        }
                    }

                    pieceSelected = false;
                    selectedX = selectedY = -1;
                    memset(validMoves, 0, sizeof(validMoves));

                    // Switch player
                    currentPlayer = (currentPlayer == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

                    // Check game state
                    inCheck = isKingInCheck(currentPlayer);
                    if (!hasLegalMoves(currentPlayer)) {
                        if (inCheck) {
                            checkmate = true;
                        } else {
                            stalemate = true;
                        }
                    }
                } else if (board[cursorX][cursorY].color == currentPlayer) {
                    // Select different piece
                    selectedX = cursorX;
                    selectedY = cursorY;
                    calculateValidMoves(cursorX, cursorY);
                } else {
                    // Deselect
                    pieceSelected = false;
                    selectedX = selectedY = -1;
                    memset(validMoves, 0, sizeof(validMoves));
                }
            }
            needsRedraw = true;
            break;

        case Button::BACK:
            if (pieceSelected) {
                pieceSelected = false;
                selectedX = selectedY = -1;
                memset(validMoves, 0, sizeof(validMoves));
                needsRedraw = true;
            }
            break;

        default:
            break;
    }
}

void Chess::draw() {
    display.clear();

    if (gameOver) {
        drawBoard();
        drawPieces();
        drawGameOverScreen();
    } else {
        drawBoard();
        drawValidMoves();
        drawPieces();
        drawCursor();
        drawStatus();
    }
}

void Chess::drawBoard() {
    // Draw board squares
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int px = BOARD_OFFSET_X + x * SQUARE_SIZE;
            int py = BOARD_OFFSET_Y + y * SQUARE_SIZE;

            if ((x + y) % 2 == 0) {
                // Light square
                display.fillRect(px, py, SQUARE_SIZE, SQUARE_SIZE, GxEPD_WHITE);
            } else {
                // Dark square - checkerboard pattern
                for (int dy = 0; dy < SQUARE_SIZE; dy += 2) {
                    for (int dx = (dy/2) % 2; dx < SQUARE_SIZE; dx += 2) {
                        display.drawPixel(px + dx, py + dy, GxEPD_BLACK);
                    }
                }
            }
        }
    }

    // Draw board border
    display.drawRect(BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2,
                     8 * SQUARE_SIZE + 4, 8 * SQUARE_SIZE + 4, GxEPD_BLACK);
    display.drawRect(BOARD_OFFSET_X - 3, BOARD_OFFSET_Y - 3,
                     8 * SQUARE_SIZE + 6, 8 * SQUARE_SIZE + 6, GxEPD_BLACK);

    // Draw rank numbers (1-8)
    display.setFont(&FreeSans9pt7b);
    for (int y = 0; y < 8; y++) {
        char rank[2] = {(char)('8' - y), '\0'};
        display.setCursor(BOARD_OFFSET_X - 20, BOARD_OFFSET_Y + y * SQUARE_SIZE + 32);
        display.print(rank);
    }

    // Draw file letters (a-h)
    for (int x = 0; x < 8; x++) {
        char file[2] = {(char)('a' + x), '\0'};
        display.setCursor(BOARD_OFFSET_X + x * SQUARE_SIZE + 20, BOARD_OFFSET_Y + 8 * SQUARE_SIZE + 18);
        display.print(file);
    }
}

void Chess::drawPieces() {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[x][y].piece != Piece::EMPTY) {
                drawPiece(x, y, board[x][y].piece, board[x][y].color);
            }
        }
    }
}

void Chess::drawPiece(int x, int y, Piece piece, PieceColor color) {
    int px = BOARD_OFFSET_X + x * SQUARE_SIZE + SQUARE_SIZE / 2;
    int py = BOARD_OFFSET_Y + y * SQUARE_SIZE + SQUARE_SIZE / 2 + 8;

    display.setFont(&FreeSansBold18pt7b);

    const char* pieceStr = getPieceChar(piece, color);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(pieceStr, 0, 0, &x1, &y1, &w, &h);

    // Draw piece with outline for visibility
    if (color == PieceColor::WHITE) {
        // White piece: filled white with black outline
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(px - w/2 - x1 - 1, py);
        display.print(pieceStr);
        display.setCursor(px - w/2 - x1 + 1, py);
        display.print(pieceStr);
        display.setCursor(px - w/2 - x1, py - 1);
        display.print(pieceStr);
        display.setCursor(px - w/2 - x1, py + 1);
        display.print(pieceStr);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(px - w/2 - x1, py);
        display.print(pieceStr);
    } else {
        // Black piece: solid black
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(px - w/2 - x1, py);
        display.print(pieceStr);
    }
}

const char* Chess::getPieceChar(Piece piece, PieceColor color) {
    switch (piece) {
        case Piece::KING:   return "K";
        case Piece::QUEEN:  return "Q";
        case Piece::ROOK:   return "R";
        case Piece::BISHOP: return "B";
        case Piece::KNIGHT: return "N";
        case Piece::PAWN:   return "P";
        default:            return "";
    }
}

void Chess::drawCursor() {
    int px = BOARD_OFFSET_X + cursorX * SQUARE_SIZE;
    int py = BOARD_OFFSET_Y + cursorY * SQUARE_SIZE;

    // Draw cursor frame
    display.drawRect(px, py, SQUARE_SIZE, SQUARE_SIZE, GxEPD_BLACK);
    display.drawRect(px + 1, py + 1, SQUARE_SIZE - 2, SQUARE_SIZE - 2, GxEPD_BLACK);
    display.drawRect(px + 2, py + 2, SQUARE_SIZE - 4, SQUARE_SIZE - 4, GxEPD_BLACK);

    // Highlight selected piece
    if (pieceSelected) {
        int sx = BOARD_OFFSET_X + selectedX * SQUARE_SIZE;
        int sy = BOARD_OFFSET_Y + selectedY * SQUARE_SIZE;

        // Draw selection indicator
        for (int i = 0; i < 4; i++) {
            display.drawRect(sx + i, sy + i, SQUARE_SIZE - 2*i, SQUARE_SIZE - 2*i, GxEPD_BLACK);
        }
    }
}

void Chess::drawValidMoves() {
    if (!pieceSelected) return;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (validMoves[x][y]) {
                int px = BOARD_OFFSET_X + x * SQUARE_SIZE + SQUARE_SIZE / 2;
                int py = BOARD_OFFSET_Y + y * SQUARE_SIZE + SQUARE_SIZE / 2;

                if (board[x][y].piece == Piece::EMPTY) {
                    // Empty square - draw dot
                    display.fillCircle(px, py, 5, GxEPD_BLACK);
                } else {
                    // Capture - draw corners
                    int s = SQUARE_SIZE;
                    int ox = BOARD_OFFSET_X + x * SQUARE_SIZE;
                    int oy = BOARD_OFFSET_Y + y * SQUARE_SIZE;

                    // Top-left corner
                    display.drawLine(ox, oy, ox + 10, oy, GxEPD_BLACK);
                    display.drawLine(ox, oy, ox, oy + 10, GxEPD_BLACK);
                    // Top-right corner
                    display.drawLine(ox + s - 10, oy, ox + s, oy, GxEPD_BLACK);
                    display.drawLine(ox + s, oy, ox + s, oy + 10, GxEPD_BLACK);
                    // Bottom-left corner
                    display.drawLine(ox, oy + s - 10, ox, oy + s, GxEPD_BLACK);
                    display.drawLine(ox, oy + s, ox + 10, oy + s, GxEPD_BLACK);
                    // Bottom-right corner
                    display.drawLine(ox + s - 10, oy + s, ox + s, oy + s, GxEPD_BLACK);
                    display.drawLine(ox + s, oy + s - 10, ox + s, oy + s, GxEPD_BLACK);
                }
            }
        }
    }
}

void Chess::drawStatus() {
    int statusX = 10;
    int statusY = 80;

    display.setFont(&FreeSansBold12pt7b);

    // Current turn
    display.setCursor(statusX, statusY);
    display.print(currentPlayer == PieceColor::WHITE ? "White" : "Black");
    display.print("'s turn");

    // Check indicator
    if (inCheck) {
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(statusX, statusY + 30);
        display.print("CHECK!");
    }

    // Controls help
    display.setFont(&FreeSans9pt7b);
    statusX = 620;
    statusY = 80;

    display.setCursor(statusX, statusY);
    display.print("Controls:");
    display.setCursor(statusX, statusY + 20);
    display.print("D-pad: Move");
    display.setCursor(statusX, statusY + 40);
    display.print("A: Select");
    display.setCursor(statusX, statusY + 60);
    display.print("B: Cancel");
}

void Chess::drawGameOverScreen() {
    const char* message;
    if (checkmate) {
        message = (currentPlayer == PieceColor::WHITE) ? "Black wins!" : "White wins!";
    } else {
        message = "Stalemate - Draw!";
    }
    drawGameOver(message);
}

void Chess::calculateValidMoves(int x, int y) {
    memset(validMoves, 0, sizeof(validMoves));

    for (int ty = 0; ty < 8; ty++) {
        for (int tx = 0; tx < 8; tx++) {
            if (isValidMove(x, y, tx, ty, true)) {
                validMoves[tx][ty] = true;
            }
        }
    }
}

bool Chess::isValidMove(int fromX, int fromY, int toX, int toY, bool checkKingSafety) {
    // Basic bounds check
    if (fromX < 0 || fromX > 7 || fromY < 0 || fromY > 7 ||
        toX < 0 || toX > 7 || toY < 0 || toY > 7) {
        return false;
    }

    // Can't move to same square
    if (fromX == toX && fromY == toY) {
        return false;
    }

    Square& from = board[fromX][fromY];
    Square& to = board[toX][toY];

    // Must have a piece to move
    if (from.piece == Piece::EMPTY) {
        return false;
    }

    // Can't capture own piece
    if (to.color == from.color) {
        return false;
    }

    // Check if piece can make this move
    if (!canPieceMove(fromX, fromY, toX, toY)) {
        return false;
    }

    // Check if move puts own king in check
    if (checkKingSafety) {
        // Make temporary move
        Square capturedPiece = to;
        to = from;
        from = {Piece::EMPTY, PieceColor::NONE, false};

        bool kingInCheck = isKingInCheck(to.color);

        // Undo temporary move
        from = to;
        to = capturedPiece;

        if (kingInCheck) {
            return false;
        }
    }

    return true;
}

bool Chess::canPieceMove(int fromX, int fromY, int toX, int toY) {
    Square& piece = board[fromX][fromY];
    int dx = toX - fromX;
    int dy = toY - fromY;

    switch (piece.piece) {
        case Piece::PAWN: {
            int direction = (piece.color == PieceColor::WHITE) ? -1 : 1;
            int startRow = (piece.color == PieceColor::WHITE) ? 6 : 1;

            // Forward move
            if (dx == 0 && board[toX][toY].piece == Piece::EMPTY) {
                if (dy == direction) {
                    return true;
                }
                // Double move from start
                if (fromY == startRow && dy == 2 * direction &&
                    board[fromX][fromY + direction].piece == Piece::EMPTY) {
                    return true;
                }
            }
            // Diagonal capture
            if (abs(dx) == 1 && dy == direction) {
                if (board[toX][toY].piece != Piece::EMPTY) {
                    return true;
                }
                // En passant
                if (toX == enPassantX && toY == enPassantY) {
                    return true;
                }
            }
            return false;
        }

        case Piece::ROOK:
            if (dx != 0 && dy != 0) return false;
            return isPathClear(fromX, fromY, toX, toY);

        case Piece::KNIGHT:
            return (abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2);

        case Piece::BISHOP:
            if (abs(dx) != abs(dy)) return false;
            return isPathClear(fromX, fromY, toX, toY);

        case Piece::QUEEN:
            if (dx != 0 && dy != 0 && abs(dx) != abs(dy)) return false;
            return isPathClear(fromX, fromY, toX, toY);

        case Piece::KING:
            // Normal king move
            if (abs(dx) <= 1 && abs(dy) <= 1) {
                return true;
            }
            // Castling
            if (dy == 0 && abs(dx) == 2 && !piece.hasMoved && !inCheck) {
                if (dx > 0 && canCastleKingside(piece.color)) return true;
                if (dx < 0 && canCastleQueenside(piece.color)) return true;
            }
            return false;

        default:
            return false;
    }
}

bool Chess::isPathClear(int fromX, int fromY, int toX, int toY) {
    int dx = (toX > fromX) ? 1 : (toX < fromX) ? -1 : 0;
    int dy = (toY > fromY) ? 1 : (toY < fromY) ? -1 : 0;

    int x = fromX + dx;
    int y = fromY + dy;

    while (x != toX || y != toY) {
        if (board[x][y].piece != Piece::EMPTY) {
            return false;
        }
        x += dx;
        y += dy;
    }
    return true;
}

void Chess::makeMove(int fromX, int fromY, int toX, int toY) {
    Square& from = board[fromX][fromY];
    Square& to = board[toX][toY];

    // Handle en passant capture
    if (from.piece == Piece::PAWN && toX == enPassantX && toY == enPassantY) {
        int captureY = (from.color == PieceColor::WHITE) ? toY + 1 : toY - 1;
        board[toX][captureY] = {Piece::EMPTY, PieceColor::NONE, false};
    }

    // Update en passant square
    enPassantX = enPassantY = -1;
    if (from.piece == Piece::PAWN && abs(toY - fromY) == 2) {
        enPassantX = fromX;
        enPassantY = (fromY + toY) / 2;
    }

    // Handle castling
    if (from.piece == Piece::KING && abs(toX - fromX) == 2) {
        if (toX > fromX) {
            // Kingside
            board[toX - 1][toY] = board[7][toY];
            board[7][toY] = {Piece::EMPTY, PieceColor::NONE, false};
            board[toX - 1][toY].hasMoved = true;
        } else {
            // Queenside
            board[toX + 1][toY] = board[0][toY];
            board[0][toY] = {Piece::EMPTY, PieceColor::NONE, false};
            board[toX + 1][toY].hasMoved = true;
        }
    }

    // Make the move
    to = from;
    to.hasMoved = true;
    from = {Piece::EMPTY, PieceColor::NONE, false};
}

bool Chess::isKingInCheck(PieceColor color) {
    int8_t kingX, kingY;
    findKing(color, kingX, kingY);

    if (kingX < 0) return false;

    PieceColor enemy = (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[x][y].color == enemy) {
                if (isValidMove(x, y, kingX, kingY, false)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Chess::hasLegalMoves(PieceColor color) {
    for (int fy = 0; fy < 8; fy++) {
        for (int fx = 0; fx < 8; fx++) {
            if (board[fx][fy].color == color) {
                for (int ty = 0; ty < 8; ty++) {
                    for (int tx = 0; tx < 8; tx++) {
                        if (isValidMove(fx, fy, tx, ty, true)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void Chess::findKing(PieceColor color, int8_t& kx, int8_t& ky) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[x][y].piece == Piece::KING && board[x][y].color == color) {
                kx = x;
                ky = y;
                return;
            }
        }
    }
    kx = ky = -1;
}

bool Chess::canCastleKingside(PieceColor color) {
    int y = (color == PieceColor::WHITE) ? 7 : 0;

    // Check if rook is in position and hasn't moved
    if (board[7][y].piece != Piece::ROOK || board[7][y].hasMoved) return false;

    // Check if path is clear
    if (board[5][y].piece != Piece::EMPTY || board[6][y].piece != Piece::EMPTY) return false;

    // Check if king passes through check
    for (int x = 4; x <= 6; x++) {
        // Temporarily place king
        Square oldSquare = board[x][y];
        board[x][y] = board[4][y];
        board[4][y] = {Piece::EMPTY, PieceColor::NONE, false};

        bool inCheck = isKingInCheck(color);

        board[4][y] = board[x][y];
        board[x][y] = oldSquare;

        if (inCheck) return false;
    }

    return true;
}

bool Chess::canCastleQueenside(PieceColor color) {
    int y = (color == PieceColor::WHITE) ? 7 : 0;

    if (board[0][y].piece != Piece::ROOK || board[0][y].hasMoved) return false;
    if (board[1][y].piece != Piece::EMPTY ||
        board[2][y].piece != Piece::EMPTY ||
        board[3][y].piece != Piece::EMPTY) return false;

    for (int x = 4; x >= 2; x--) {
        Square oldSquare = board[x][y];
        board[x][y] = board[4][y];
        board[4][y] = {Piece::EMPTY, PieceColor::NONE, false};

        bool inCheck = isKingInCheck(color);

        board[4][y] = board[x][y];
        board[x][y] = oldSquare;

        if (inCheck) return false;
    }

    return true;
}

void Chess::handlePromotion(int x, int y) {
    // For simplicity, always promote to queen
    // A full implementation would show a selection menu
    board[x][y].piece = Piece::QUEEN;
}

// Simple AI using minimax with alpha-beta pruning
Move Chess::findBestMove(PieceColor color, int depth) {
    Move bestMove = {{-1}, {-1}, {-1}, {-1}, Piece::EMPTY, false, -999999};

    for (int fy = 0; fy < 8; fy++) {
        for (int fx = 0; fx < 8; fx++) {
            if (board[fx][fy].color == color) {
                for (int ty = 0; ty < 8; ty++) {
                    for (int tx = 0; tx < 8; tx++) {
                        if (isValidMove(fx, fy, tx, ty, true)) {
                            // Make move
                            Square captured = board[tx][ty];
                            board[tx][ty] = board[fx][fy];
                            board[fx][fy] = {Piece::EMPTY, PieceColor::NONE, false};

                            int score = -minimax(depth - 1, -999999, 999999, false);

                            // Undo move
                            board[fx][fy] = board[tx][ty];
                            board[tx][ty] = captured;

                            if (score > bestMove.score) {
                                bestMove = {(int8_t)fx, (int8_t)fy, (int8_t)tx, (int8_t)ty, Piece::EMPTY, true, score};
                            }
                        }
                    }
                }
            }
        }
    }

    return bestMove;
}

int Chess::minimax(int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) {
        return evaluateBoard();
    }

    PieceColor color = maximizing ?
        (currentPlayer == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE) :
        currentPlayer;

    int bestScore = maximizing ? -999999 : 999999;

    for (int fy = 0; fy < 8; fy++) {
        for (int fx = 0; fx < 8; fx++) {
            if (board[fx][fy].color == color) {
                for (int ty = 0; ty < 8; ty++) {
                    for (int tx = 0; tx < 8; tx++) {
                        if (isValidMove(fx, fy, tx, ty, true)) {
                            Square captured = board[tx][ty];
                            board[tx][ty] = board[fx][fy];
                            board[fx][fy] = {Piece::EMPTY, PieceColor::NONE, false};

                            int score = minimax(depth - 1, alpha, beta, !maximizing);

                            board[fx][fy] = board[tx][ty];
                            board[tx][ty] = captured;

                            if (maximizing) {
                                bestScore = max(bestScore, score);
                                alpha = max(alpha, score);
                            } else {
                                bestScore = min(bestScore, score);
                                beta = min(beta, score);
                            }

                            if (beta <= alpha) break;
                        }
                    }
                    if (beta <= alpha) break;
                }
            }
            if (beta <= alpha) break;
        }
        if (beta <= alpha) break;
    }

    return bestScore;
}

int Chess::evaluateBoard() {
    int score = 0;
    PieceColor aiColor = (currentPlayer == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[x][y].piece != Piece::EMPTY) {
                int value = getPieceValue(board[x][y].piece);
                if (board[x][y].color == aiColor) {
                    score += value;
                } else {
                    score -= value;
                }
            }
        }
    }

    return score;
}

int Chess::getPieceValue(Piece piece) {
    switch (piece) {
        case Piece::PAWN:   return 100;
        case Piece::KNIGHT: return 320;
        case Piece::BISHOP: return 330;
        case Piece::ROOK:   return 500;
        case Piece::QUEEN:  return 900;
        case Piece::KING:   return 20000;
        default:            return 0;
    }
}
