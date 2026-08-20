/**
 * X4 Games - Main Entry Point
 *
 * A collection of games for the Xteink X4 e-paper reader
 * Compatible with Papyrix firmware partition layout
 *
 * FLASHING NOTES:
 * - The checked-in partition table follows the Papyrix layout
 * - Back up the device and flash the application image at 0x10000
 * - Verify the partition layout against the exact hardware revision first
 *
 * Controls:
 * - D-PAD: Navigate menus and game controls
 * - CONFIRM (A): Select / Action
 * - BACK (B): Cancel / Back
 * - POWER: Special actions (varies by game)
 */

#include <Arduino.h>
#include <esp_sleep.h>
#include "display.h"
#include "input.h"
#include "menu.h"

// Include all games
#include "games/chess.h"
#include "games/minesweeper.h"
#include "games/snake.h"
#include "games/game2048.h"
#include "games/gameoflife.h"

// Include apps/utilities
#include "apps/stocktracker.h"

#if __has_include("config_local.h")
#include "config_local.h"
#else
static void configureLocalPortfolio(StockTracker&) {}
#endif

// Game instances
Chess chessGame;
Minesweeper minesweeperGame;
Snake snakeGame;
Game2048 game2048;
GameOfLife gameOfLife;

// App instances
StockTracker stockTracker;

void drawSplashScreen() {
    display.clear();

    // Draw border
    display.drawRect(50, 50, DISPLAY_WIDTH - 100, DISPLAY_HEIGHT - 100, GxEPD_BLACK);
    display.drawRect(52, 52, DISPLAY_WIDTH - 104, DISPLAY_HEIGHT - 104, GxEPD_BLACK);

    // Title
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.drawCenteredText("X4 GAMES", 150);

    // Subtitle
    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("A collection of classic games", 200);
    display.drawCenteredText("for the Xteink X4", 230);

    // Version
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("Version 1.0.0", 280);

    // Games list
    display.setFont(&FreeSansBold12pt7b);
    display.drawCenteredText("Included:", 320);

    display.setFont(&FreeSans12pt7b);
    const char* items[] = {"Chess", "Minesweeper", "Snake", "2048", "Game of Life", "Portfolio Tracker"};
    int startY = 345;
    for (int i = 0; i < 6; i++) {
        char line[32];
        snprintf(line, sizeof(line), "%d. %s", i + 1, items[i]);
        display.drawCenteredText(line, startY + i * 20);
    }

    // Press to continue
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("Press any button to continue...", DISPLAY_HEIGHT - 70);

    display.fullRefresh();

    // Wait for button press
    input.waitForButton();
}

void setup() {
    // Initialize serial for debugging (optional)
    Serial.begin(115200);
    Serial.println("X4 Games starting...");

    // Initialize display
    display.begin();
    Serial.println("Display initialized");

    // Initialize input
    input.begin();
    Serial.println("Input initialized");

    // Show splash screen
    drawSplashScreen();

    // Register all games with menu
    gameMenu.addGame(&chessGame);
    gameMenu.addGame(&minesweeperGame);
    gameMenu.addGame(&snakeGame);
    gameMenu.addGame(&game2048);
    gameMenu.addGame(&gameOfLife);

    // Register apps/utilities. Personal values live in the ignored local config.
    configureLocalPortfolio(stockTracker);
    gameMenu.addGame(&stockTracker);

    Serial.println("Setup complete!");
}

void loop() {
    // Show game menu
    int selected = gameMenu.show();

    if (selected >= 0) {
        // Launch selected game
        Game* game = gameMenu.getGame(selected);
        if (game) {
            Serial.printf("Launching game: %s\n", game->name());
            game->run();
            Serial.println("Game exited");
        }
    } else {
        // User pressed back/power from menu
        // Show goodbye screen and enter deep sleep
        display.clear();

        display.setFont(&FreeSansBold18pt7b);
        display.drawCenteredText("Goodbye!", 200);

        display.setFont(&FreeSans12pt7b);
        display.drawCenteredText("Press POWER to wake up", 250);

        display.fullRefresh();

        // Wait a moment then sleep
        delay(2000);

        // Enter deep sleep - wake on power button
        const esp_err_t wakeStatus = esp_deep_sleep_enable_gpio_wakeup(
            1ULL << BTN_POWER_PIN,
            ESP_GPIO_WAKEUP_GPIO_LOW
        );

        if (wakeStatus == ESP_OK) {
            esp_deep_sleep_start();
        } else {
            Serial.printf("Unable to configure power-button wake-up: %d\n", wakeStatus);
            delay(1000);
        }
    }
}
