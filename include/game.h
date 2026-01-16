/**
 * Game Base Class
 * Abstract base class for all games
 */

#ifndef GAME_H
#define GAME_H

#include "display.h"
#include "input.h"

class Game {
public:
    virtual ~Game() = default;

    // Game name for menu
    virtual const char* name() const = 0;

    // Short description
    virtual const char* description() const = 0;

    // Initialize/reset game state
    virtual void init() = 0;

    // Main game loop - returns false when game should exit
    virtual bool update() = 0;

    // Draw the current game state
    virtual void draw() = 0;

    // Handle input - called by update()
    virtual void handleInput(Button btn) = 0;

    // Check if game is over
    virtual bool isGameOver() const { return gameOver; }

    // Run the game (main loop)
    void run();

protected:
    bool gameOver = false;
    bool needsRedraw = true;
    bool exitRequested = false;

    // Helper to draw a standard game header
    void drawHeader(const char* title, int score = -1);

    // Helper to draw game over screen
    void drawGameOver(const char* message, int score = -1);

    // Helper to draw pause menu
    bool drawPauseMenu();
};

#endif // GAME_H
