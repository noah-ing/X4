/**
 * Conway's Game of Life Implementation
 */

#include "games/gameoflife.h"
#include <stdlib.h>
#include <string.h>

void GameOfLife::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    running = false;
    editing = true;
    generation = 0;
    population = 0;
    currentBuffer = 0;
    showingPatternMenu = false;
    selectedPattern = 0;

    cursorX = GRID_WIDTH / 2;
    cursorY = GRID_HEIGHT / 2;

    clearGrid();

    // Start with a few gliders
    placeGlider(GRID_WIDTH / 4, GRID_HEIGHT / 4);
    placeGlider(GRID_WIDTH * 3 / 4, GRID_HEIGHT / 4);
    placeGlider(GRID_WIDTH / 2, GRID_HEIGHT / 2);
}

void GameOfLife::clearGrid() {
    memset(cells, 0, sizeof(cells));
    generation = 0;
    population = 0;
}

void GameOfLife::randomize() {
    srand(millis());
    clearGrid();

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            cells[currentBuffer][x][y] = (rand() % 100) < 25; // 25% alive
            if (cells[currentBuffer][x][y]) population++;
        }
    }
}

int GameOfLife::countNeighbors(int x, int y) {
    int count = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            // Wrap around edges (toroidal)
            int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
            int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;

            if (cells[currentBuffer][nx][ny]) count++;
        }
    }

    return count;
}

void GameOfLife::step() {
    int nextBuffer = 1 - currentBuffer;
    population = 0;

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int neighbors = countNeighbors(x, y);
            bool alive = cells[currentBuffer][x][y];

            // Conway's rules:
            // 1. Live cell with 2-3 neighbors survives
            // 2. Dead cell with exactly 3 neighbors becomes alive
            // 3. All other cells die or stay dead

            if (alive) {
                cells[nextBuffer][x][y] = (neighbors == 2 || neighbors == 3);
            } else {
                cells[nextBuffer][x][y] = (neighbors == 3);
            }

            if (cells[nextBuffer][x][y]) population++;
        }
    }

    currentBuffer = nextBuffer;
    generation++;
}

bool GameOfLife::update() {
    if (running && !editing && !showingPatternMenu) {
        uint32_t now = millis();
        if (now - lastStepTime >= stepInterval) {
            step();
            lastStepTime = now;
            needsRedraw = true;
        }
    }

    return !exitRequested;
}

void GameOfLife::handleInput(Button btn) {
    if (showingPatternMenu) {
        switch (btn) {
            case Button::UP:
                if (selectedPattern > 0) selectedPattern--;
                needsRedraw = true;
                break;
            case Button::DOWN:
                if (selectedPattern < 8) selectedPattern++;
                needsRedraw = true;
                break;
            case Button::CONFIRM:
                loadPattern((Pattern)selectedPattern);
                showingPatternMenu = false;
                needsRedraw = true;
                break;
            case Button::BACK:
                showingPatternMenu = false;
                needsRedraw = true;
                break;
            default:
                break;
        }
        return;
    }

    switch (btn) {
        case Button::LEFT:
            if (editing) {
                cursorX = (cursorX - 1 + GRID_WIDTH) % GRID_WIDTH;
                needsRedraw = true;
            }
            break;

        case Button::RIGHT:
            if (editing) {
                cursorX = (cursorX + 1) % GRID_WIDTH;
                needsRedraw = true;
            }
            break;

        case Button::UP:
            if (editing) {
                cursorY = (cursorY - 1 + GRID_HEIGHT) % GRID_HEIGHT;
                needsRedraw = true;
            } else {
                // Speed up
                if (stepInterval > 50) {
                    stepInterval -= 50;
                    needsRedraw = true;
                }
            }
            break;

        case Button::DOWN:
            if (editing) {
                cursorY = (cursorY + 1) % GRID_HEIGHT;
                needsRedraw = true;
            } else {
                // Slow down
                if (stepInterval < 1000) {
                    stepInterval += 50;
                    needsRedraw = true;
                }
            }
            break;

        case Button::CONFIRM:
            if (editing) {
                // Toggle cell
                cells[currentBuffer][cursorX][cursorY] = !cells[currentBuffer][cursorX][cursorY];
                if (cells[currentBuffer][cursorX][cursorY]) {
                    population++;
                } else {
                    population--;
                }
                needsRedraw = true;
            } else {
                // Manual step
                step();
                needsRedraw = true;
            }
            break;

        case Button::BACK:
            if (running) {
                running = false;
                editing = true;
            } else {
                // Show pattern menu
                showingPatternMenu = true;
            }
            needsRedraw = true;
            break;

        case Button::POWER:
            // Toggle run/pause
            running = !running;
            editing = !running;
            lastStepTime = millis();
            needsRedraw = true;
            break;

        default:
            break;
    }
}

void GameOfLife::loadPattern(Pattern pattern) {
    switch (pattern) {
        case Pattern::CLEAR:
            clearGrid();
            break;
        case Pattern::RANDOM:
            randomize();
            break;
        case Pattern::GLIDER:
            clearGrid();
            placeGlider(GRID_WIDTH / 2, GRID_HEIGHT / 2);
            break;
        case Pattern::BLINKER:
            clearGrid();
            placeBlinker(GRID_WIDTH / 2, GRID_HEIGHT / 2);
            break;
        case Pattern::BEACON:
            clearGrid();
            placeBeacon(GRID_WIDTH / 2, GRID_HEIGHT / 2);
            break;
        case Pattern::PULSAR:
            clearGrid();
            placePulsar(GRID_WIDTH / 2, GRID_HEIGHT / 2);
            break;
        case Pattern::GLIDER_GUN:
            clearGrid();
            placeGliderGun(5, GRID_HEIGHT / 2 - 5);
            break;
        case Pattern::SPACESHIP:
            clearGrid();
            placeSpaceship(GRID_WIDTH / 2, GRID_HEIGHT / 2);
            break;
        case Pattern::ACORN:
            clearGrid();
            placeAcorn(GRID_WIDTH / 2, GRID_HEIGHT / 2);
            break;
    }

    // Recalculate population
    population = 0;
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (cells[currentBuffer][x][y]) population++;
        }
    }

    generation = 0;
}

void GameOfLife::placeGlider(int x, int y) {
    // Glider pattern
    //  X
    //   X
    // XXX
    cells[currentBuffer][(x + 1) % GRID_WIDTH][y % GRID_HEIGHT] = true;
    cells[currentBuffer][(x + 2) % GRID_WIDTH][(y + 1) % GRID_HEIGHT] = true;
    cells[currentBuffer][x % GRID_WIDTH][(y + 2) % GRID_HEIGHT] = true;
    cells[currentBuffer][(x + 1) % GRID_WIDTH][(y + 2) % GRID_HEIGHT] = true;
    cells[currentBuffer][(x + 2) % GRID_WIDTH][(y + 2) % GRID_HEIGHT] = true;
}

void GameOfLife::placeBlinker(int x, int y) {
    cells[currentBuffer][x % GRID_WIDTH][y % GRID_HEIGHT] = true;
    cells[currentBuffer][(x + 1) % GRID_WIDTH][y % GRID_HEIGHT] = true;
    cells[currentBuffer][(x + 2) % GRID_WIDTH][y % GRID_HEIGHT] = true;
}

void GameOfLife::placeBeacon(int x, int y) {
    // Beacon pattern
    // XX
    // X
    //    X
    //   XX
    cells[currentBuffer][x][y] = true;
    cells[currentBuffer][x + 1][y] = true;
    cells[currentBuffer][x][y + 1] = true;
    cells[currentBuffer][x + 3][y + 2] = true;
    cells[currentBuffer][x + 2][y + 3] = true;
    cells[currentBuffer][x + 3][y + 3] = true;
}

void GameOfLife::placePulsar(int x, int y) {
    // Pulsar - period 3 oscillator
    int dx[] = {-6,-5,-4, -2,-1, 1,2, 4,5,6};
    int dy[] = {-4,-1, 1,4};

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            int px = (x + dx[i] + GRID_WIDTH) % GRID_WIDTH;
            int py = (y + dy[j] + GRID_HEIGHT) % GRID_HEIGHT;
            cells[currentBuffer][px][py] = true;
        }
    }

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 10; i++) {
            int px = (x + dy[j] + GRID_WIDTH) % GRID_WIDTH;
            int py = (y + dx[i] + GRID_HEIGHT) % GRID_HEIGHT;
            cells[currentBuffer][px][py] = true;
        }
    }
}

void GameOfLife::placeGliderGun(int x, int y) {
    // Gosper Glider Gun
    const int pattern[][2] = {
        {0,4},{0,5},{1,4},{1,5},
        {10,4},{10,5},{10,6},{11,3},{11,7},{12,2},{12,8},{13,2},{13,8},
        {14,5},{15,3},{15,7},{16,4},{16,5},{16,6},{17,5},
        {20,2},{20,3},{20,4},{21,2},{21,3},{21,4},{22,1},{22,5},
        {24,0},{24,1},{24,5},{24,6},
        {34,2},{34,3},{35,2},{35,3}
    };

    int patternSize = sizeof(pattern) / sizeof(pattern[0]);
    for (int i = 0; i < patternSize; i++) {
        int px = (x + pattern[i][0]) % GRID_WIDTH;
        int py = (y + pattern[i][1]) % GRID_HEIGHT;
        cells[currentBuffer][px][py] = true;
    }
}

void GameOfLife::placeSpaceship(int x, int y) {
    // Lightweight spaceship (LWSS)
    const int pattern[][2] = {
        {1,0},{4,0},{0,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3}
    };

    int patternSize = sizeof(pattern) / sizeof(pattern[0]);
    for (int i = 0; i < patternSize; i++) {
        int px = (x + pattern[i][0]) % GRID_WIDTH;
        int py = (y + pattern[i][1]) % GRID_HEIGHT;
        cells[currentBuffer][px][py] = true;
    }
}

void GameOfLife::placeAcorn(int x, int y) {
    // Acorn - grows to 633 cells after 5206 generations
    const int pattern[][2] = {
        {1,0},{3,1},{0,2},{1,2},{4,2},{5,2},{6,2}
    };

    int patternSize = sizeof(pattern) / sizeof(pattern[0]);
    for (int i = 0; i < patternSize; i++) {
        int px = (x + pattern[i][0]) % GRID_WIDTH;
        int py = (y + pattern[i][1]) % GRID_HEIGHT;
        cells[currentBuffer][px][py] = true;
    }
}

void GameOfLife::draw() {
    display.clear();

    if (showingPatternMenu) {
        drawPatternMenu();
    } else {
        drawGrid();
        if (editing) {
            drawCursor();
        }
        drawStatus();
    }
}

void GameOfLife::drawGrid() {
    // Draw border
    display.drawRect(BOARD_OFFSET_X - 1, BOARD_OFFSET_Y - 1,
                     GRID_WIDTH * CELL_SIZE + 2, GRID_HEIGHT * CELL_SIZE + 2, GxEPD_BLACK);

    // Draw cells
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (cells[currentBuffer][x][y]) {
                int px = BOARD_OFFSET_X + x * CELL_SIZE;
                int py = BOARD_OFFSET_Y + y * CELL_SIZE;
                display.fillRect(px, py, CELL_SIZE - 1, CELL_SIZE - 1, GxEPD_BLACK);
            }
        }
    }
}

void GameOfLife::drawCursor() {
    int px = BOARD_OFFSET_X + cursorX * CELL_SIZE;
    int py = BOARD_OFFSET_Y + cursorY * CELL_SIZE;

    // Draw cursor box
    display.drawRect(px - 1, py - 1, CELL_SIZE + 1, CELL_SIZE + 1, GxEPD_BLACK);
    display.drawRect(px - 2, py - 2, CELL_SIZE + 3, CELL_SIZE + 3, GxEPD_BLACK);
}

void GameOfLife::drawStatus() {
    // Header
    display.fillRect(0, 0, DISPLAY_WIDTH, 48, GxEPD_BLACK);

    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(20, 35);
    display.print("GAME OF LIFE");

    // Generation
    char genText[32];
    snprintf(genText, sizeof(genText), "Gen: %d", generation);
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(280, 32);
    display.print(genText);

    // Population
    snprintf(genText, sizeof(genText), "Pop: %d", population);
    display.setCursor(450, 32);
    display.print(genText);

    // Speed
    snprintf(genText, sizeof(genText), "%dms", (int)stepInterval);
    display.setCursor(620, 32);
    display.print(genText);

    // State indicator
    display.setFont(&FreeSans9pt7b);
    display.setCursor(720, 42);
    if (running) {
        display.print("RUN");
    } else if (editing) {
        display.print("EDIT");
    } else {
        display.print("PAUSE");
    }

    display.setTextColor(GxEPD_BLACK);

    // Controls
    int y = DISPLAY_HEIGHT - 20;
    display.setFont(&FreeSans9pt7b);

    if (editing) {
        display.setCursor(20, y);
        display.print("D-PAD: Move cursor");
        display.setCursor(250, y);
        display.print("A: Toggle cell");
        display.setCursor(450, y);
        display.print("B: Patterns");
        display.setCursor(620, y);
        display.print("PWR: Run");
    } else {
        display.setCursor(20, y);
        display.print("UP/DOWN: Speed");
        display.setCursor(200, y);
        display.print("A: Step");
        display.setCursor(350, y);
        display.print("B: Edit mode");
        display.setCursor(550, y);
        display.print("PWR: Pause");
    }
}

void GameOfLife::drawPatternMenu() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 60, GxEPD_BLACK);
    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.drawCenteredText("SELECT PATTERN", 42);
    display.setTextColor(GxEPD_BLACK);

    const char* patterns[] = {
        "Clear Grid",
        "Random",
        "Glider",
        "Blinker (Oscillator)",
        "Beacon (Oscillator)",
        "Pulsar (Oscillator)",
        "Gosper Glider Gun",
        "Spaceship (LWSS)",
        "Acorn (Methuselah)"
    };

    int menuY = 80;
    int itemHeight = 40;

    for (int i = 0; i < 9; i++) {
        int y = menuY + i * itemHeight;
        bool selected = (i == selectedPattern);

        if (selected) {
            display.fillRoundRect(100, y, 600, 35, 6, GxEPD_BLACK);
            display.setTextColor(GxEPD_WHITE);
        } else {
            display.drawRoundRect(100, y, 600, 35, 6, GxEPD_BLACK);
            display.setTextColor(GxEPD_BLACK);
        }

        display.setFont(&FreeSans12pt7b);
        display.drawCenteredText(patterns[i], 100, y + 25, 600);
    }

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("UP/DOWN: Select    A: Load    B: Cancel", DISPLAY_HEIGHT - 20);
}
