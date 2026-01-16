/**
 * 2048 Game for X4
 * Slide tiles to combine numbers and reach 2048
 */

#ifndef GAME2048_H
#define GAME2048_H

#include "game.h"

class Game2048 : public Game {
public:
    const char* name() const override { return "2048"; }
    const char* description() const override { return "Slide tiles to reach 2048!"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

private:
    // Board is 4x4
    static constexpr int GRID_SIZE = 4;
    static constexpr int TILE_SIZE = 90;
    static constexpr int TILE_GAP = 8;

    // Calculate board offset for centering
    static constexpr int BOARD_SIZE = GRID_SIZE * TILE_SIZE + (GRID_SIZE + 1) * TILE_GAP;
    static constexpr int BOARD_OFFSET_X = (DISPLAY_WIDTH - BOARD_SIZE) / 2;
    static constexpr int BOARD_OFFSET_Y = 60;

    // Game board
    int board[GRID_SIZE][GRID_SIZE];
    int score = 0;
    int highScore = 0;
    int highestTile = 0;
    bool won = false;
    bool canContinue = true;

    // Add a new tile (2 or 4)
    void addRandomTile();

    // Move tiles in direction
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();

    // Helper for sliding and merging
    bool slideRow(int row[GRID_SIZE]);

    // Check if any moves possible
    bool canMove() const;

    // Drawing
    void drawBoard();
    void drawTile(int x, int y, int value);
    void drawStatus();
    void drawWinScreen();

    // Get pattern for tile based on value
    void getTileStyle(int value, bool& filled, bool& inverted, int& pattern);
};

#endif // GAME2048_H
