/**
 * Game Base Class Implementation
 */

#include "game.h"

void Game::run() {
    init();
    needsRedraw = true;

    while (!exitRequested) {
        if (needsRedraw) {
            draw();
            display.fullRefresh();
            needsRedraw = false;
        }

        input.update();
        Button btn = input.getPressed();

        if (btn == Button::BACK && !gameOver) {
            if (drawPauseMenu()) {
                exitRequested = true;
                break;
            }
            needsRedraw = true;
        } else if (btn != Button::NONE) {
            handleInput(btn);
            if (!update()) {
                break;
            }
        }

        delay(20);
    }
}

void Game::drawHeader(const char* title, int score) {
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_BLACK);

    // Draw header background
    display.fillRect(0, 0, DISPLAY_WIDTH, 40, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    // Draw title
    display.setCursor(10, 28);
    display.print(title);

    // Draw score if provided
    if (score >= 0) {
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "Score: %d", score);

        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(scoreText, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(DISPLAY_WIDTH - w - 15, 28);
        display.print(scoreText);
    }

    display.setTextColor(GxEPD_BLACK);

    // Draw separator line
    display.drawLine(0, 42, DISPLAY_WIDTH, 42, GxEPD_BLACK);
}

void Game::drawGameOver(const char* message, int score) {
    // Semi-transparent overlay effect (checkerboard pattern)
    for (int y = 100; y < 380; y += 2) {
        for (int x = 100 + (y % 4); x < 700; x += 4) {
            display.drawPixel(x, y, GxEPD_BLACK);
        }
    }

    // Draw dialog box
    int boxX = 150, boxY = 150, boxW = 500, boxH = 180;
    display.fillRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_WHITE);
    display.drawRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_BLACK);
    display.drawRoundRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, 8, GxEPD_BLACK);

    // Game Over text
    display.setFont(&FreeSansBold24pt7b);
    display.drawCenteredText("GAME OVER", boxX, boxY + 55, boxW);

    // Message
    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText(message, boxX, boxY + 95, boxW);

    // Score if provided
    if (score >= 0) {
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "Final Score: %d", score);
        display.setFont(&FreeSansBold12pt7b);
        display.drawCenteredText(scoreText, boxX, boxY + 125, boxW);
    }

    // Instructions
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("Press CONFIRM to play again, BACK to exit", boxX, boxY + 160, boxW);
}

bool Game::drawPauseMenu() {
    // Draw pause overlay
    int boxX = 200, boxY = 150, boxW = 400, boxH = 180;
    display.fillRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_WHITE);
    display.drawRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_BLACK);
    display.drawRoundRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, 8, GxEPD_BLACK);

    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("PAUSED", boxX, boxY + 50, boxW);

    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("CONFIRM - Resume", boxX, boxY + 100, boxW);
    display.drawCenteredText("BACK - Exit Game", boxX, boxY + 135, boxW);

    display.partialRefresh();

    // Wait for input
    while (true) {
        input.update();
        Button btn = input.getPressed();

        if (btn == Button::CONFIRM) {
            return false; // Resume
        }
        if (btn == Button::BACK) {
            return true; // Exit
        }
        delay(20);
    }
}
