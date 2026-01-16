/**
 * Game Menu Implementation
 */

#include "menu.h"

Menu gameMenu;

void Menu::addGame(Game* game) {
    if (numGames < MAX_GAMES && game != nullptr) {
        games[numGames++] = game;
    }
}

Game* Menu::getGame(int index) {
    if (index >= 0 && index < numGames) {
        return games[index];
    }
    return nullptr;
}

int Menu::show() {
    selectedIndex = 0;
    scrollOffset = 0;

    draw();
    display.fullRefresh();

    while (true) {
        input.update();
        Button btn = input.getPressed();

        bool needsRedraw = false;

        switch (btn) {
            case Button::UP:
            case Button::LEFT:
                if (selectedIndex > 0) {
                    selectedIndex--;
                    if (selectedIndex < scrollOffset) {
                        scrollOffset = selectedIndex;
                    }
                    needsRedraw = true;
                }
                break;

            case Button::DOWN:
            case Button::RIGHT:
                if (selectedIndex < numGames - 1) {
                    selectedIndex++;
                    if (selectedIndex >= scrollOffset + ITEMS_PER_PAGE) {
                        scrollOffset = selectedIndex - ITEMS_PER_PAGE + 1;
                    }
                    needsRedraw = true;
                }
                break;

            case Button::CONFIRM:
                return selectedIndex;

            case Button::BACK:
            case Button::POWER:
                return -1;

            default:
                break;
        }

        if (needsRedraw) {
            draw();
            display.partialRefresh();
        }

        delay(20);
    }
}

void Menu::draw() {
    display.clear();
    drawHeader();

    // Draw visible game items
    int yPos = MENU_START_Y;
    for (int i = 0; i < ITEMS_PER_PAGE && (scrollOffset + i) < numGames; i++) {
        int gameIndex = scrollOffset + i;
        drawGameItem(gameIndex, yPos, gameIndex == selectedIndex);
        yPos += ITEM_HEIGHT;
    }

    drawScrollIndicators();
    drawControls();
}

void Menu::drawHeader() {
    // Title background
    display.fillRect(0, 0, DISPLAY_WIDTH, 70, GxEPD_BLACK);

    // Title text
    display.setFont(&FreeSansBold24pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.drawCenteredText("X4 GAMES", 50);

    // Subtitle
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("Select a game to play", 68);

    display.setTextColor(GxEPD_BLACK);

    // Decorative line
    display.drawLine(0, 75, DISPLAY_WIDTH, 75, GxEPD_BLACK);
    display.drawLine(0, 77, DISPLAY_WIDTH, 77, GxEPD_BLACK);
}

void Menu::drawGameItem(int index, int yPos, bool selected) {
    Game* game = games[index];
    if (!game) return;

    int boxX = 40;
    int boxW = DISPLAY_WIDTH - 80;
    int boxH = ITEM_HEIGHT - 10;

    if (selected) {
        // Selected item - filled background
        display.fillRoundRect(boxX, yPos, boxW, boxH, 8, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);

        // Selection arrow
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(10, yPos + 45);
        display.print(">");
    } else {
        // Unselected item - outline only
        display.drawRoundRect(boxX, yPos, boxW, boxH, 8, GxEPD_BLACK);
        display.setTextColor(GxEPD_BLACK);
    }

    // Game number
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(boxX + 15, yPos + 35);
    display.print(index + 1);
    display.print(".");

    // Game name
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(boxX + 55, yPos + 38);
    display.print(game->name());

    // Game description
    display.setFont(&FreeSans9pt7b);
    display.setCursor(boxX + 55, yPos + 60);
    display.print(game->description());

    display.setTextColor(GxEPD_BLACK);
}

void Menu::drawScrollIndicators() {
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_BLACK);

    // Up arrow if can scroll up
    if (scrollOffset > 0) {
        display.setCursor(DISPLAY_WIDTH - 40, 95);
        display.print("^");
    }

    // Down arrow if can scroll down
    if (scrollOffset + ITEMS_PER_PAGE < numGames) {
        display.setCursor(DISPLAY_WIDTH - 40, MENU_START_Y + ITEMS_PER_PAGE * ITEM_HEIGHT - 10);
        display.print("v");
    }

    // Page indicator
    int currentPage = (selectedIndex / ITEMS_PER_PAGE) + 1;
    int totalPages = ((numGames - 1) / ITEMS_PER_PAGE) + 1;

    if (totalPages > 1) {
        char pageText[16];
        snprintf(pageText, sizeof(pageText), "%d/%d", currentPage, totalPages);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(DISPLAY_WIDTH - 50, 95);
        display.print(pageText);
    }
}

void Menu::drawControls() {
    int y = DISPLAY_HEIGHT - 25;

    display.drawLine(0, y - 15, DISPLAY_WIDTH, y - 15, GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);

    display.setCursor(50, y);
    display.print("UP/DOWN: Navigate");

    display.setCursor(300, y);
    display.print("CONFIRM: Select");

    display.setCursor(550, y);
    display.print("BACK: Exit");
}
