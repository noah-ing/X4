/**
 * Chess Game for X4
 * Full chess implementation with AI opponent
 */

#ifndef CHESS_H
#define CHESS_H

#include "game.h"

// Piece types
enum class Piece : uint8_t {
    EMPTY = 0,
    PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
};

// Piece colors
enum class PieceColor : uint8_t {
    NONE = 0,
    WHITE,
    BLACK
};

// Square representation
struct Square {
    Piece piece = Piece::EMPTY;
    PieceColor color = PieceColor::NONE;
    bool hasMoved = false;  // For castling and pawn double move
};

// Move representation
struct Move {
    int8_t fromX, fromY;
    int8_t toX, toY;
    Piece promotion = Piece::EMPTY;
    bool isValid = false;
    int score = 0;  // For AI evaluation
};

class Chess : public Game {
public:
    const char* name() const override { return "Chess"; }
    const char* description() const override { return "Classic chess vs AI opponent"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

private:
    // Board state
    Square board[8][8];
    PieceColor currentPlayer = PieceColor::WHITE;
    bool playerIsWhite = true;

    // Cursor and selection
    int8_t cursorX = 0, cursorY = 0;
    int8_t selectedX = -1, selectedY = -1;
    bool pieceSelected = false;

    // Valid moves for selected piece
    bool validMoves[8][8];

    // Game state
    bool inCheck = false;
    bool checkmate = false;
    bool stalemate = false;
    Move lastMove;

    // En passant tracking
    int8_t enPassantX = -1, enPassantY = -1;

    // Drawing constants
    static constexpr int BOARD_OFFSET_X = 160;
    static constexpr int BOARD_OFFSET_Y = 50;
    static constexpr int SQUARE_SIZE = 50;

    // Board setup
    void setupBoard();

    // Move validation
    bool isValidMove(int fromX, int fromY, int toX, int toY, bool checkKingSafety = true);
    void calculateValidMoves(int x, int y);
    bool canPieceMove(int fromX, int fromY, int toX, int toY);
    bool isPathClear(int fromX, int fromY, int toX, int toY);

    // Move execution
    void makeMove(int fromX, int fromY, int toX, int toY);
    void undoMove(const Move& move, Square capturedPiece);

    // Check detection
    bool isKingInCheck(PieceColor color);
    bool hasLegalMoves(PieceColor color);
    void findKing(PieceColor color, int8_t& kx, int8_t& ky);

    // Special moves
    bool canCastleKingside(PieceColor color);
    bool canCastleQueenside(PieceColor color);
    void handlePromotion(int x, int y);

    // AI
    Move findBestMove(PieceColor color, int depth);
    int evaluateBoard();
    int minimax(int depth, int alpha, int beta, bool maximizing);
    int getPieceValue(Piece piece);

    // Drawing
    void drawBoard();
    void drawPieces();
    void drawPiece(int x, int y, Piece piece, PieceColor color);
    void drawCursor();
    void drawValidMoves();
    void drawStatus();
    void drawMoveHistory();
    void drawGameOverScreen();

    // Piece characters for display
    const char* getPieceChar(Piece piece, PieceColor color);
};

#endif // CHESS_H
