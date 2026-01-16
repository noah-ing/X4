/**
 * Minesweeper Game for X4
 * Classic mine-finding puzzle game
 */

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include "game.h"

// Cell states
enum class CellState : uint8_t {
    HIDDEN,
    REVEALED,
    FLAGGED
};

// Difficulty levels
enum class Difficulty : uint8_t {
    EASY,       // 9x9, 10 mines
    MEDIUM,     // 16x16, 40 mines
    HARD        // 20x12, 50 mines (fits screen)
};

class Minesweeper : public Game {
public:
    const char* name() const override { return "Minesweeper"; }
    const char* description() const override { return "Find all mines without exploding"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

private:
    // Board dimensions (max size)
    static constexpr int MAX_WIDTH = 20;
    static constexpr int MAX_HEIGHT = 12;

    // Current board settings
    int boardWidth = 16;
    int boardHeight = 10;
    int mineCount = 30;
    Difficulty difficulty = Difficulty::MEDIUM;

    // Board data
    bool mines[MAX_WIDTH][MAX_HEIGHT];
    int adjacentMines[MAX_WIDTH][MAX_HEIGHT];
    CellState cellState[MAX_WIDTH][MAX_HEIGHT];

    // Game state
    int cursorX = 0, cursorY = 0;
    int flagsPlaced = 0;
    int cellsRevealed = 0;
    bool firstClick = true;
    bool won = false;
    bool exploded = false;
    int explosionX = -1, explosionY = -1;
    uint32_t startTime = 0;
    uint32_t endTime = 0;

    // Menu state
    bool showingDifficultyMenu = false;

    // Drawing constants
    int cellSize = 32;
    int boardOffsetX = 0;
    int boardOffsetY = 60;

    // Setup
    void setupBoard();
    void placeMines(int safeX, int safeY);
    void calculateAdjacent();

    // Gameplay
    void revealCell(int x, int y);
    void revealAllMines();
    void floodReveal(int x, int y);
    bool checkWin();

    // Drawing
    void drawBoard();
    void drawCell(int x, int y);
    void drawCursor();
    void drawStatus();
    void drawDifficultyMenu();
    void drawWinScreen();
    void drawLoseScreen();

    // Helpers
    void setDifficulty(Difficulty diff);
    int countAdjacentFlags(int x, int y);
    void chordReveal(int x, int y);
};

#endif // MINESWEEPER_H
