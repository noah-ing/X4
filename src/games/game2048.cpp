/**
 * 2048 Game Implementation
 */

#include "games/game2048.h"
#include <stdlib.h>
#include <string.h>

void Game2048::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    score = 0;
    won = false;
    canContinue = true;
    highestTile = 0;

    // Clear board
    memset(board, 0, sizeof(board));

    // Add two starting tiles
    addRandomTile();
    addRandomTile();
}

void Game2048::addRandomTile() {
    // Count empty cells
    int emptyCells[GRID_SIZE * GRID_SIZE][2];
    int emptyCount = 0;

    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (board[x][y] == 0) {
                emptyCells[emptyCount][0] = x;
                emptyCells[emptyCount][1] = y;
                emptyCount++;
            }
        }
    }

    if (emptyCount == 0) return;

    // Pick random empty cell
    srand(millis());
    int idx = rand() % emptyCount;
    int x = emptyCells[idx][0];
    int y = emptyCells[idx][1];

    // 90% chance of 2, 10% chance of 4
    board[x][y] = (rand() % 10 < 9) ? 2 : 4;
}

bool Game2048::slideRow(int row[GRID_SIZE]) {
    bool moved = false;
    int temp[GRID_SIZE] = {0};
    int writePos = 0;

    // First pass: slide non-zero values to the left
    for (int i = 0; i < GRID_SIZE; i++) {
        if (row[i] != 0) {
            if (writePos != i) moved = true;
            temp[writePos++] = row[i];
        }
    }

    // Second pass: merge adjacent equal values
    for (int i = 0; i < GRID_SIZE - 1; i++) {
        if (temp[i] != 0 && temp[i] == temp[i + 1]) {
            temp[i] *= 2;
            score += temp[i];
            if (temp[i] > highestTile) highestTile = temp[i];
            if (temp[i] == 2048 && !won) {
                won = true;
            }
            // Shift remaining values left
            for (int j = i + 1; j < GRID_SIZE - 1; j++) {
                temp[j] = temp[j + 1];
            }
            temp[GRID_SIZE - 1] = 0;
            moved = true;
        }
    }

    // Copy back
    for (int i = 0; i < GRID_SIZE; i++) {
        if (row[i] != temp[i]) moved = true;
        row[i] = temp[i];
    }

    return moved;
}

bool Game2048::moveLeft() {
    bool moved = false;
    for (int y = 0; y < GRID_SIZE; y++) {
        int row[GRID_SIZE];
        for (int x = 0; x < GRID_SIZE; x++) {
            row[x] = board[x][y];
        }
        if (slideRow(row)) moved = true;
        for (int x = 0; x < GRID_SIZE; x++) {
            board[x][y] = row[x];
        }
    }
    return moved;
}

bool Game2048::moveRight() {
    bool moved = false;
    for (int y = 0; y < GRID_SIZE; y++) {
        int row[GRID_SIZE];
        // Reverse for right movement
        for (int x = 0; x < GRID_SIZE; x++) {
            row[x] = board[GRID_SIZE - 1 - x][y];
        }
        if (slideRow(row)) moved = true;
        for (int x = 0; x < GRID_SIZE; x++) {
            board[GRID_SIZE - 1 - x][y] = row[x];
        }
    }
    return moved;
}

bool Game2048::moveUp() {
    bool moved = false;
    for (int x = 0; x < GRID_SIZE; x++) {
        int col[GRID_SIZE];
        for (int y = 0; y < GRID_SIZE; y++) {
            col[y] = board[x][y];
        }
        if (slideRow(col)) moved = true;
        for (int y = 0; y < GRID_SIZE; y++) {
            board[x][y] = col[y];
        }
    }
    return moved;
}

bool Game2048::moveDown() {
    bool moved = false;
    for (int x = 0; x < GRID_SIZE; x++) {
        int col[GRID_SIZE];
        for (int y = 0; y < GRID_SIZE; y++) {
            col[y] = board[x][GRID_SIZE - 1 - y];
        }
        if (slideRow(col)) moved = true;
        for (int y = 0; y < GRID_SIZE; y++) {
            board[x][GRID_SIZE - 1 - y] = col[y];
        }
    }
    return moved;
}

bool Game2048::canMove() const {
    // Check for empty cells
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (board[x][y] == 0) return true;
        }
    }

    // Check for adjacent equal values
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            if (x < GRID_SIZE - 1 && board[x][y] == board[x + 1][y]) return true;
            if (y < GRID_SIZE - 1 && board[x][y] == board[x][y + 1]) return true;
        }
    }

    return false;
}

bool Game2048::update() {
    return !exitRequested;
}

void Game2048::handleInput(Button btn) {
    if (gameOver) {
        if (btn == Button::CONFIRM) {
            if (score > highScore) highScore = score;
            init();
        } else if (btn == Button::BACK) {
            exitRequested = true;
        }
        return;
    }

    // Win screen - option to continue
    if (won && canContinue) {
        if (btn == Button::CONFIRM) {
            canContinue = false; // Don't show win screen again
            needsRedraw = true;
            return;
        } else if (btn == Button::BACK) {
            if (score > highScore) highScore = score;
            init();
            return;
        }
        return;
    }

    bool moved = false;

    switch (btn) {
        case Button::UP:
            moved = moveUp();
            break;
        case Button::DOWN:
            moved = moveDown();
            break;
        case Button::LEFT:
            moved = moveLeft();
            break;
        case Button::RIGHT:
            moved = moveRight();
            break;
        case Button::BACK:
            // New game
            if (score > highScore) highScore = score;
            init();
            needsRedraw = true;
            return;
        default:
            break;
    }

    if (moved) {
        addRandomTile();
        needsRedraw = true;

        if (!canMove()) {
            gameOver = true;
            if (score > highScore) highScore = score;
        }
    }
}

void Game2048::draw() {
    display.clear();

    drawStatus();
    drawBoard();

    if (gameOver) {
        drawGameOver("No more moves!", score);
    } else if (won && canContinue) {
        drawWinScreen();
    }
}

void Game2048::drawBoard() {
    // Board background
    display.fillRoundRect(BOARD_OFFSET_X, BOARD_OFFSET_Y,
                          BOARD_SIZE, BOARD_SIZE, 8, GxEPD_BLACK);

    // Draw tiles
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            drawTile(x, y, board[x][y]);
        }
    }
}

void Game2048::getTileStyle(int value, bool& filled, bool& inverted, int& pattern) {
    // Different visual styles for different tile values
    // pattern: 0 = solid, 1 = light dots, 2 = heavy dots, 3 = lines
    filled = false;
    inverted = false;
    pattern = 0;

    switch (value) {
        case 0:    filled = false; inverted = false; pattern = 0; break;
        case 2:    filled = false; inverted = false; pattern = 0; break;
        case 4:    filled = false; inverted = false; pattern = 1; break;
        case 8:    filled = false; inverted = false; pattern = 2; break;
        case 16:   filled = false; inverted = false; pattern = 3; break;
        case 32:   filled = true;  inverted = true;  pattern = 1; break;
        case 64:   filled = true;  inverted = true;  pattern = 2; break;
        case 128:  filled = true;  inverted = true;  pattern = 0; break;
        case 256:  filled = true;  inverted = true;  pattern = 0; break;
        case 512:  filled = true;  inverted = true;  pattern = 0; break;
        case 1024: filled = true;  inverted = true;  pattern = 0; break;
        case 2048: filled = true;  inverted = true;  pattern = 0; break;
        default:   filled = true;  inverted = true;  pattern = 0; break;
    }
}

void Game2048::drawTile(int x, int y, int value) {
    int px = BOARD_OFFSET_X + TILE_GAP + x * (TILE_SIZE + TILE_GAP);
    int py = BOARD_OFFSET_Y + TILE_GAP + y * (TILE_SIZE + TILE_GAP);

    bool filled, inverted;
    int pattern;
    getTileStyle(value, filled, inverted, pattern);

    // Draw tile background
    if (value == 0) {
        // Empty tile - subtle pattern
        display.fillRoundRect(px, py, TILE_SIZE, TILE_SIZE, 6, GxEPD_WHITE);
        display.drawRoundRect(px, py, TILE_SIZE, TILE_SIZE, 6, GxEPD_BLACK);
    } else if (filled) {
        // Filled tile (high values)
        display.fillRoundRect(px, py, TILE_SIZE, TILE_SIZE, 6, GxEPD_BLACK);

        // Apply pattern on top
        if (pattern == 1) {
            for (int dy = 4; dy < TILE_SIZE - 4; dy += 4) {
                for (int dx = 4 + (dy/4) % 2 * 2; dx < TILE_SIZE - 4; dx += 4) {
                    display.drawPixel(px + dx, py + dy, GxEPD_WHITE);
                }
            }
        } else if (pattern == 2) {
            for (int dy = 3; dy < TILE_SIZE - 3; dy += 3) {
                for (int dx = 3 + (dy/3) % 2; dx < TILE_SIZE - 3; dx += 3) {
                    display.drawPixel(px + dx, py + dy, GxEPD_WHITE);
                }
            }
        }
    } else {
        // Outline tile (low values)
        display.fillRoundRect(px, py, TILE_SIZE, TILE_SIZE, 6, GxEPD_WHITE);

        // Apply pattern
        if (pattern == 1) {
            for (int dy = 4; dy < TILE_SIZE - 4; dy += 6) {
                for (int dx = 4 + (dy/6) % 2 * 3; dx < TILE_SIZE - 4; dx += 6) {
                    display.drawPixel(px + dx, py + dy, GxEPD_BLACK);
                }
            }
        } else if (pattern == 2) {
            for (int dy = 3; dy < TILE_SIZE - 3; dy += 4) {
                for (int dx = 3 + (dy/4) % 2 * 2; dx < TILE_SIZE - 3; dx += 4) {
                    display.drawPixel(px + dx, py + dy, GxEPD_BLACK);
                }
            }
        } else if (pattern == 3) {
            for (int dy = 4; dy < TILE_SIZE - 4; dy += 4) {
                display.drawLine(px + 4, py + dy, px + TILE_SIZE - 4, py + dy, GxEPD_BLACK);
            }
        }

        display.drawRoundRect(px, py, TILE_SIZE, TILE_SIZE, 6, GxEPD_BLACK);
        display.drawRoundRect(px + 1, py + 1, TILE_SIZE - 2, TILE_SIZE - 2, 5, GxEPD_BLACK);
    }

    // Draw number
    if (value > 0) {
        char numStr[8];
        snprintf(numStr, sizeof(numStr), "%d", value);

        // Choose font size based on number of digits
        if (value >= 1000) {
            display.setFont(&FreeSansBold12pt7b);
        } else if (value >= 100) {
            display.setFont(&FreeSansBold18pt7b);
        } else {
            display.setFont(&FreeSansBold24pt7b);
        }

        display.setTextColor(inverted ? GxEPD_WHITE : GxEPD_BLACK);

        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(numStr, 0, 0, &x1, &y1, &w, &h);

        int textX = px + (TILE_SIZE - w) / 2 - x1;
        int textY = py + (TILE_SIZE + h) / 2;

        display.setCursor(textX, textY);
        display.print(numStr);
    }

    display.setTextColor(GxEPD_BLACK);
}

void Game2048::drawStatus() {
    // Title
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(20, 38);
    display.print("2048");

    // Score
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(150, 25);
    display.print("Score");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(150, 45);
    display.print(score);

    // Best
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(320, 25);
    display.print("Best");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(320, 45);
    display.print(highScore);

    // Highest tile
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(480, 25);
    display.print("Max Tile");
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(480, 45);
    display.print(highestTile);

    display.setTextColor(GxEPD_BLACK);

    // Controls
    display.setFont(&FreeSans9pt7b);
    int y = DISPLAY_HEIGHT - 20;
    display.setCursor(20, y);
    display.print("D-PAD: Slide tiles");
    display.setCursor(300, y);
    display.print("Goal: Reach 2048!");
    display.setCursor(550, y);
    display.print("B: New game");
}

void Game2048::drawWinScreen() {
    // Overlay
    int boxX = 150, boxY = 150, boxW = 500, boxH = 180;
    display.fillRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_WHITE);
    display.drawRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_BLACK);
    display.drawRoundRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, 8, GxEPD_BLACK);

    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.drawCenteredText("YOU WIN!", boxX, boxY + 55, boxW);

    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("2048!", boxX, boxY + 95, boxW);

    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("A: Keep playing   B: New game", boxX, boxY + 145, boxW);
}
