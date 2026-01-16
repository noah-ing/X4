/**
 * Conway's Game of Life for X4
 * Cellular automaton simulation
 */

#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include "game.h"

class GameOfLife : public Game {
public:
    const char* name() const override { return "Game of Life"; }
    const char* description() const override { return "Conway's cellular automaton"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

private:
    // Grid dimensions
    static constexpr int GRID_WIDTH = 80;
    static constexpr int GRID_HEIGHT = 48;
    static constexpr int CELL_SIZE = 8;

    // Board offset
    static constexpr int BOARD_OFFSET_X = (DISPLAY_WIDTH - GRID_WIDTH * CELL_SIZE) / 2;
    static constexpr int BOARD_OFFSET_Y = 55;

    // Double buffer for simulation
    bool cells[2][GRID_WIDTH][GRID_HEIGHT];
    int currentBuffer = 0;

    // State
    int cursorX = GRID_WIDTH / 2;
    int cursorY = GRID_HEIGHT / 2;
    bool running = false;
    bool editing = true;
    int generation = 0;
    int population = 0;
    uint32_t lastStepTime = 0;
    uint32_t stepInterval = 200; // ms between generations

    // Preset patterns
    enum class Pattern {
        CLEAR,
        RANDOM,
        GLIDER,
        BLINKER,
        BEACON,
        PULSAR,
        GLIDER_GUN,
        SPACESHIP,
        ACORN
    };

    // Simulation
    void step();
    int countNeighbors(int x, int y);

    // Pattern loading
    void loadPattern(Pattern pattern);
    void clearGrid();
    void randomize();

    // Drawing
    void drawGrid();
    void drawCursor();
    void drawStatus();
    void drawPatternMenu();

    // Pattern data
    void placeGlider(int x, int y);
    void placeBlinker(int x, int y);
    void placeBeacon(int x, int y);
    void placePulsar(int x, int y);
    void placeGliderGun(int x, int y);
    void placeSpaceship(int x, int y);
    void placeAcorn(int x, int y);

    // State
    bool showingPatternMenu = false;
    int selectedPattern = 0;
};

#endif // GAMEOFLIFE_H
