/**
 * Game Menu System for X4 Games
 * Main launcher and game selection interface
 */

#ifndef MENU_H
#define MENU_H

#include "display.h"
#include "input.h"
#include "game.h"

// Maximum number of games supported
#define MAX_GAMES 8

class Menu {
public:
    // Register a game with the menu
    void addGame(Game* game);

    // Show the menu and return selected game index (or -1 to exit)
    int show();

    // Draw the menu
    void draw();

    // Get game count
    int gameCount() const { return numGames; }

    // Get game by index
    Game* getGame(int index);

private:
    Game* games[MAX_GAMES] = {nullptr};
    int numGames = 0;
    int selectedIndex = 0;
    int scrollOffset = 0;

    static constexpr int ITEMS_PER_PAGE = 4;
    static constexpr int ITEM_HEIGHT = 90;
    static constexpr int MENU_START_Y = 100;

    void drawGameItem(int index, int yPos, bool selected);
    void drawHeader();
    void drawScrollIndicators();
    void drawControls();
};

extern Menu gameMenu;

#endif // MENU_H
