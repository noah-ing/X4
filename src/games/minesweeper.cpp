/**
 * Minesweeper Implementation
 */

#include "games/minesweeper.h"
#include <stdlib.h>

void Minesweeper::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    showingDifficultyMenu = true;

    setDifficulty(Difficulty::MEDIUM);
}

void Minesweeper::setDifficulty(Difficulty diff) {
    difficulty = diff;

    switch (diff) {
        case Difficulty::EASY:
            boardWidth = 9;
            boardHeight = 9;
            mineCount = 10;
            cellSize = 40;
            break;
        case Difficulty::MEDIUM:
            boardWidth = 16;
            boardHeight = 10;
            mineCount = 30;
            cellSize = 36;
            break;
        case Difficulty::HARD:
            boardWidth = 20;
            boardHeight = 12;
            mineCount = 50;
            cellSize = 32;
            break;
    }

    // Center the board
    boardOffsetX = (DISPLAY_WIDTH - boardWidth * cellSize) / 2;
    boardOffsetY = 60;

    setupBoard();
}

void Minesweeper::setupBoard() {
    // Reset board
    for (int y = 0; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < MAX_WIDTH; x++) {
            mines[x][y] = false;
            adjacentMines[x][y] = 0;
            cellState[x][y] = CellState::HIDDEN;
        }
    }

    cursorX = boardWidth / 2;
    cursorY = boardHeight / 2;
    flagsPlaced = 0;
    cellsRevealed = 0;
    firstClick = true;
    won = false;
    exploded = false;
    explosionX = explosionY = -1;
    startTime = 0;
    endTime = 0;
}

void Minesweeper::placeMines(int safeX, int safeY) {
    srand(millis());
    int placed = 0;

    while (placed < mineCount) {
        int x = rand() % boardWidth;
        int y = rand() % boardHeight;

        // Don't place mine on or adjacent to first click
        if (abs(x - safeX) <= 1 && abs(y - safeY) <= 1) {
            continue;
        }

        if (!mines[x][y]) {
            mines[x][y] = true;
            placed++;
        }
    }

    calculateAdjacent();
}

void Minesweeper::calculateAdjacent() {
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth; x++) {
            if (mines[x][y]) {
                adjacentMines[x][y] = -1;
                continue;
            }

            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < boardWidth && ny >= 0 && ny < boardHeight) {
                        if (mines[nx][ny]) count++;
                    }
                }
            }
            adjacentMines[x][y] = count;
        }
    }
}

bool Minesweeper::update() {
    return !exitRequested;
}

void Minesweeper::handleInput(Button btn) {
    if (showingDifficultyMenu) {
        switch (btn) {
            case Button::UP:
                if (difficulty != Difficulty::EASY) {
                    setDifficulty((Difficulty)((int)difficulty - 1));
                }
                needsRedraw = true;
                break;
            case Button::DOWN:
                if (difficulty != Difficulty::HARD) {
                    setDifficulty((Difficulty)((int)difficulty + 1));
                }
                needsRedraw = true;
                break;
            case Button::CONFIRM:
                showingDifficultyMenu = false;
                needsRedraw = true;
                break;
            case Button::BACK:
                exitRequested = true;
                break;
            default:
                break;
        }
        return;
    }

    if (gameOver) {
        if (btn == Button::CONFIRM) {
            showingDifficultyMenu = true;
            needsRedraw = true;
        } else if (btn == Button::BACK) {
            exitRequested = true;
        }
        return;
    }

    switch (btn) {
        case Button::LEFT:
            if (cursorX > 0) cursorX--;
            needsRedraw = true;
            break;

        case Button::RIGHT:
            if (cursorX < boardWidth - 1) cursorX++;
            needsRedraw = true;
            break;

        case Button::UP:
            if (cursorY > 0) cursorY--;
            needsRedraw = true;
            break;

        case Button::DOWN:
            if (cursorY < boardHeight - 1) cursorY++;
            needsRedraw = true;
            break;

        case Button::CONFIRM:
            // Reveal cell
            if (cellState[cursorX][cursorY] == CellState::HIDDEN) {
                if (firstClick) {
                    firstClick = false;
                    startTime = millis();
                    placeMines(cursorX, cursorY);
                }
                revealCell(cursorX, cursorY);
                needsRedraw = true;
            } else if (cellState[cursorX][cursorY] == CellState::REVEALED) {
                // Chord reveal if enough flags around
                chordReveal(cursorX, cursorY);
                needsRedraw = true;
            }
            break;

        case Button::BACK:
            // Toggle flag
            if (cellState[cursorX][cursorY] == CellState::HIDDEN) {
                cellState[cursorX][cursorY] = CellState::FLAGGED;
                flagsPlaced++;
                needsRedraw = true;
            } else if (cellState[cursorX][cursorY] == CellState::FLAGGED) {
                cellState[cursorX][cursorY] = CellState::HIDDEN;
                flagsPlaced--;
                needsRedraw = true;
            }
            break;

        default:
            break;
    }

    // Check win condition
    if (!gameOver && checkWin()) {
        won = true;
        gameOver = true;
        endTime = millis();
        needsRedraw = true;
    }
}

void Minesweeper::revealCell(int x, int y) {
    if (x < 0 || x >= boardWidth || y < 0 || y >= boardHeight) return;
    if (cellState[x][y] != CellState::HIDDEN) return;

    cellState[x][y] = CellState::REVEALED;
    cellsRevealed++;

    if (mines[x][y]) {
        exploded = true;
        explosionX = x;
        explosionY = y;
        gameOver = true;
        endTime = millis();
        revealAllMines();
        return;
    }

    // Flood fill for empty cells
    if (adjacentMines[x][y] == 0) {
        floodReveal(x, y);
    }
}

void Minesweeper::floodReveal(int x, int y) {
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < boardWidth && ny >= 0 && ny < boardHeight) {
                if (cellState[nx][ny] == CellState::HIDDEN) {
                    cellState[nx][ny] = CellState::REVEALED;
                    cellsRevealed++;
                    if (adjacentMines[nx][ny] == 0) {
                        floodReveal(nx, ny);
                    }
                }
            }
        }
    }
}

void Minesweeper::revealAllMines() {
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth; x++) {
            if (mines[x][y]) {
                cellState[x][y] = CellState::REVEALED;
            }
        }
    }
}

int Minesweeper::countAdjacentFlags(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < boardWidth && ny >= 0 && ny < boardHeight) {
                if (cellState[nx][ny] == CellState::FLAGGED) count++;
            }
        }
    }
    return count;
}

void Minesweeper::chordReveal(int x, int y) {
    if (adjacentMines[x][y] <= 0) return;

    int flags = countAdjacentFlags(x, y);
    if (flags != adjacentMines[x][y]) return;

    // Reveal all non-flagged adjacent cells
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < boardWidth && ny >= 0 && ny < boardHeight) {
                if (cellState[nx][ny] == CellState::HIDDEN) {
                    revealCell(nx, ny);
                }
            }
        }
    }
}

bool Minesweeper::checkWin() {
    int totalCells = boardWidth * boardHeight;
    int safeCells = totalCells - mineCount;
    return cellsRevealed == safeCells;
}

void Minesweeper::draw() {
    display.clear();

    if (showingDifficultyMenu) {
        drawDifficultyMenu();
    } else if (gameOver) {
        drawBoard();
        drawCursor();
        if (won) {
            drawWinScreen();
        } else {
            drawLoseScreen();
        }
    } else {
        drawBoard();
        drawCursor();
        drawStatus();
    }
}

void Minesweeper::drawBoard() {
    // Draw border
    display.drawRect(boardOffsetX - 2, boardOffsetY - 2,
                     boardWidth * cellSize + 4, boardHeight * cellSize + 4, GxEPD_BLACK);

    // Draw cells
    for (int y = 0; y < boardHeight; y++) {
        for (int x = 0; x < boardWidth; x++) {
            drawCell(x, y);
        }
    }

    // Draw grid lines
    for (int x = 0; x <= boardWidth; x++) {
        display.drawLine(boardOffsetX + x * cellSize, boardOffsetY,
                         boardOffsetX + x * cellSize, boardOffsetY + boardHeight * cellSize,
                         GxEPD_BLACK);
    }
    for (int y = 0; y <= boardHeight; y++) {
        display.drawLine(boardOffsetX, boardOffsetY + y * cellSize,
                         boardOffsetX + boardWidth * cellSize, boardOffsetY + y * cellSize,
                         GxEPD_BLACK);
    }
}

void Minesweeper::drawCell(int x, int y) {
    int px = boardOffsetX + x * cellSize;
    int py = boardOffsetY + y * cellSize;

    switch (cellState[x][y]) {
        case CellState::HIDDEN:
            // Filled cell (unrevealed)
            for (int dy = 1; dy < cellSize - 1; dy += 2) {
                for (int dx = 1 + (dy/2) % 2; dx < cellSize - 1; dx += 2) {
                    display.drawPixel(px + dx, py + dy, GxEPD_BLACK);
                }
            }
            break;

        case CellState::FLAGGED:
            // Flag symbol
            for (int dy = 1; dy < cellSize - 1; dy += 2) {
                for (int dx = 1 + (dy/2) % 2; dx < cellSize - 1; dx += 2) {
                    display.drawPixel(px + dx, py + dy, GxEPD_BLACK);
                }
            }
            // Draw flag
            display.setFont(&FreeSansBold12pt7b);
            display.setTextColor(GxEPD_WHITE);
            display.setCursor(px + cellSize/2 - 5, py + cellSize/2 + 6);
            display.print("F");
            display.setTextColor(GxEPD_BLACK);
            break;

        case CellState::REVEALED:
            if (mines[x][y]) {
                // Mine
                if (x == explosionX && y == explosionY) {
                    // Exploded mine - fill cell black
                    display.fillRect(px + 1, py + 1, cellSize - 2, cellSize - 2, GxEPD_BLACK);
                    display.setTextColor(GxEPD_WHITE);
                } else {
                    display.setTextColor(GxEPD_BLACK);
                }
                // Draw mine symbol
                display.fillCircle(px + cellSize/2, py + cellSize/2, cellSize/4, GxEPD_BLACK);
                display.drawLine(px + 4, py + cellSize/2, px + cellSize - 4, py + cellSize/2, GxEPD_BLACK);
                display.drawLine(px + cellSize/2, py + 4, px + cellSize/2, py + cellSize - 4, GxEPD_BLACK);
            } else if (adjacentMines[x][y] > 0) {
                // Number
                char num[2] = {(char)('0' + adjacentMines[x][y]), '\0'};
                display.setFont(&FreeSansBold12pt7b);
                display.setTextColor(GxEPD_BLACK);

                int16_t x1, y1;
                uint16_t w, h;
                display.getTextBounds(num, 0, 0, &x1, &y1, &w, &h);
                display.setCursor(px + (cellSize - w) / 2 - x1, py + (cellSize + h) / 2);
                display.print(num);
            }
            // Empty cells are just white (already cleared)
            break;
    }
}

void Minesweeper::drawCursor() {
    int px = boardOffsetX + cursorX * cellSize;
    int py = boardOffsetY + cursorY * cellSize;

    // Draw thick cursor border
    for (int i = 0; i < 3; i++) {
        display.drawRect(px - i, py - i, cellSize + 2*i, cellSize + 2*i, GxEPD_BLACK);
    }
}

void Minesweeper::drawStatus() {
    // Header
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.drawCenteredText("MINESWEEPER", 35);
    display.setTextColor(GxEPD_BLACK);

    // Status bar
    int statusY = DISPLAY_HEIGHT - 35;
    display.drawLine(0, statusY - 5, DISPLAY_WIDTH, statusY - 5, GxEPD_BLACK);

    display.setFont(&FreeSans12pt7b);

    // Mines remaining
    char mineText[32];
    snprintf(mineText, sizeof(mineText), "Mines: %d", mineCount - flagsPlaced);
    display.setCursor(20, statusY + 15);
    display.print(mineText);

    // Timer
    uint32_t elapsed = 0;
    if (startTime > 0) {
        elapsed = ((endTime > 0 ? endTime : millis()) - startTime) / 1000;
    }
    char timeText[32];
    snprintf(timeText, sizeof(timeText), "Time: %d:%02d", (int)(elapsed / 60), (int)(elapsed % 60));
    display.setCursor(DISPLAY_WIDTH / 2 - 50, statusY + 15);
    display.print(timeText);

    // Controls hint
    display.setFont(&FreeSans9pt7b);
    display.setCursor(DISPLAY_WIDTH - 180, statusY + 15);
    display.print("A:Reveal B:Flag");
}

void Minesweeper::drawDifficultyMenu() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 70, GxEPD_BLACK);
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.drawCenteredText("MINESWEEPER", 50);
    display.setTextColor(GxEPD_BLACK);

    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("Select Difficulty", 100);

    int menuY = 150;
    int menuHeight = 60;
    const char* diffNames[] = {"Easy (9x9, 10 mines)", "Medium (16x10, 30 mines)", "Hard (20x12, 50 mines)"};

    for (int i = 0; i < 3; i++) {
        int y = menuY + i * menuHeight;
        bool selected = (int)difficulty == i;

        if (selected) {
            display.fillRoundRect(150, y, 500, 50, 8, GxEPD_BLACK);
            display.setTextColor(GxEPD_WHITE);
        } else {
            display.drawRoundRect(150, y, 500, 50, 8, GxEPD_BLACK);
            display.setTextColor(GxEPD_BLACK);
        }

        display.setFont(&FreeSansBold12pt7b);
        display.drawCenteredText(diffNames[i], 150, y + 32, 500);
    }

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("UP/DOWN: Select    CONFIRM: Start    BACK: Exit", DISPLAY_HEIGHT - 30);
}

void Minesweeper::drawWinScreen() {
    uint32_t elapsed = (endTime - startTime) / 1000;
    char timeStr[32];
    snprintf(timeStr, sizeof(timeStr), "Time: %d:%02d", (int)(elapsed / 60), (int)(elapsed % 60));

    drawGameOver(timeStr, -1);

    // Override the message
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.drawCenteredText("YOU WIN!", 150, 205, 500);
}

void Minesweeper::drawLoseScreen() {
    drawGameOver("Better luck next time!");
}
