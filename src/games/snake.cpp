/**
 * Snake Game Implementation
 */

#include "games/snake.h"
#include <stdlib.h>

void Snake::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    score = 0;
    autoMove = false;
    lastMoveTime = 0;

    resetSnake();
    spawnFood();
}

void Snake::resetSnake() {
    snakeLength = 3;
    direction = Direction::RIGHT;
    nextDirection = Direction::RIGHT;

    // Start in middle of grid
    int startX = GRID_WIDTH / 2;
    int startY = GRID_HEIGHT / 2;

    // Initialize snake body (head first)
    for (int i = 0; i < snakeLength; i++) {
        snakeBody[i] = {(int16_t)(startX - i), (int16_t)startY};
    }
}

void Snake::spawnFood() {
    srand(millis());

    do {
        food.x = rand() % GRID_WIDTH;
        food.y = rand() % GRID_HEIGHT;
    } while (isSnakeBody(food.x, food.y));
}

bool Snake::isSnakeBody(int x, int y, bool includeHead) const {
    int start = includeHead ? 0 : 1;
    for (int i = start; i < snakeLength; i++) {
        if (snakeBody[i].x == x && snakeBody[i].y == y) {
            return true;
        }
    }
    return false;
}

bool Snake::checkCollision(const Point& head) const {
    // Wall collision
    if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT) {
        return true;
    }

    // Self collision (check against body, not head)
    return isSnakeBody(head.x, head.y, false);
}

void Snake::moveSnake() {
    // Apply the buffered direction
    direction = nextDirection;

    // Calculate new head position
    Point newHead = snakeBody[0];
    switch (direction) {
        case Direction::UP:    newHead.y--; break;
        case Direction::DOWN:  newHead.y++; break;
        case Direction::LEFT:  newHead.x--; break;
        case Direction::RIGHT: newHead.x++; break;
    }

    // Check collision
    if (checkCollision(newHead)) {
        gameOver = true;
        if (score > highScore) {
            highScore = score;
        }
        needsRedraw = true;
        return;
    }

    // Check if eating food
    bool ate = (newHead.x == food.x && newHead.y == food.y);

    // Move body (shift everything down)
    if (!ate) {
        // Remove tail
        for (int i = snakeLength - 1; i > 0; i--) {
            snakeBody[i] = snakeBody[i - 1];
        }
    } else {
        // Grow: shift and keep tail
        if (snakeLength < MAX_SNAKE_LENGTH) {
            for (int i = snakeLength; i > 0; i--) {
                snakeBody[i] = snakeBody[i - 1];
            }
            snakeLength++;
            score += 10;
            spawnFood();
        }
    }

    // Add new head
    snakeBody[0] = newHead;
    needsRedraw = true;
}

bool Snake::update() {
    // Auto-move mode (optional - can be toggled)
    if (autoMove && !gameOver) {
        uint32_t now = millis();
        if (now - lastMoveTime >= AUTO_MOVE_INTERVAL) {
            moveSnake();
            lastMoveTime = now;
        }
    }

    return !exitRequested;
}

void Snake::handleInput(Button btn) {
    if (gameOver) {
        if (btn == Button::CONFIRM) {
            init();
        } else if (btn == Button::BACK) {
            exitRequested = true;
        }
        return;
    }

    switch (btn) {
        case Button::UP:
            if (direction != Direction::DOWN) {
                nextDirection = Direction::UP;
                if (!autoMove) moveSnake();
            }
            break;

        case Button::DOWN:
            if (direction != Direction::UP) {
                nextDirection = Direction::DOWN;
                if (!autoMove) moveSnake();
            }
            break;

        case Button::LEFT:
            if (direction != Direction::RIGHT) {
                nextDirection = Direction::LEFT;
                if (!autoMove) moveSnake();
            }
            break;

        case Button::RIGHT:
            if (direction != Direction::LEFT) {
                nextDirection = Direction::RIGHT;
                if (!autoMove) moveSnake();
            }
            break;

        case Button::CONFIRM:
            // Move forward in current direction (for turn-based play)
            if (!autoMove) {
                moveSnake();
            }
            break;

        case Button::BACK:
            // Toggle auto-move mode
            autoMove = !autoMove;
            lastMoveTime = millis();
            needsRedraw = true;
            break;

        default:
            break;
    }
}

void Snake::draw() {
    display.clear();

    drawGrid();
    drawFood();
    drawSnake();
    drawStatus();

    if (gameOver) {
        drawGameOver("You crashed!", score);
    }
}

void Snake::drawGrid() {
    // Draw border
    display.drawRect(BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2,
                     GRID_WIDTH * CELL_SIZE + 4, GRID_HEIGHT * CELL_SIZE + 4, GxEPD_BLACK);
    display.drawRect(BOARD_OFFSET_X - 3, BOARD_OFFSET_Y - 3,
                     GRID_WIDTH * CELL_SIZE + 6, GRID_HEIGHT * CELL_SIZE + 6, GxEPD_BLACK);

    // Light grid lines (dotted)
    for (int x = 1; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT * CELL_SIZE; y += 4) {
            display.drawPixel(BOARD_OFFSET_X + x * CELL_SIZE, BOARD_OFFSET_Y + y, GxEPD_BLACK);
        }
    }
    for (int y = 1; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH * CELL_SIZE; x += 4) {
            display.drawPixel(BOARD_OFFSET_X + x, BOARD_OFFSET_Y + y * CELL_SIZE, GxEPD_BLACK);
        }
    }
}

void Snake::drawSnake() {
    for (int i = 0; i < snakeLength; i++) {
        int px = BOARD_OFFSET_X + snakeBody[i].x * CELL_SIZE;
        int py = BOARD_OFFSET_Y + snakeBody[i].y * CELL_SIZE;

        if (i == 0) {
            // Head - filled with eyes
            display.fillRect(px + 2, py + 2, CELL_SIZE - 4, CELL_SIZE - 4, GxEPD_BLACK);

            // Eyes (white dots)
            int eyeOffset = CELL_SIZE / 4;
            switch (direction) {
                case Direction::UP:
                    display.fillCircle(px + eyeOffset + 2, py + eyeOffset + 2, 3, GxEPD_WHITE);
                    display.fillCircle(px + CELL_SIZE - eyeOffset - 2, py + eyeOffset + 2, 3, GxEPD_WHITE);
                    break;
                case Direction::DOWN:
                    display.fillCircle(px + eyeOffset + 2, py + CELL_SIZE - eyeOffset - 2, 3, GxEPD_WHITE);
                    display.fillCircle(px + CELL_SIZE - eyeOffset - 2, py + CELL_SIZE - eyeOffset - 2, 3, GxEPD_WHITE);
                    break;
                case Direction::LEFT:
                    display.fillCircle(px + eyeOffset + 2, py + eyeOffset + 2, 3, GxEPD_WHITE);
                    display.fillCircle(px + eyeOffset + 2, py + CELL_SIZE - eyeOffset - 2, 3, GxEPD_WHITE);
                    break;
                case Direction::RIGHT:
                    display.fillCircle(px + CELL_SIZE - eyeOffset - 2, py + eyeOffset + 2, 3, GxEPD_WHITE);
                    display.fillCircle(px + CELL_SIZE - eyeOffset - 2, py + CELL_SIZE - eyeOffset - 2, 3, GxEPD_WHITE);
                    break;
            }
        } else {
            // Body segments - rounded rectangle with gap
            display.fillRoundRect(px + 3, py + 3, CELL_SIZE - 6, CELL_SIZE - 6, 4, GxEPD_BLACK);
        }
    }
}

void Snake::drawFood() {
    int px = BOARD_OFFSET_X + food.x * CELL_SIZE + CELL_SIZE / 2;
    int py = BOARD_OFFSET_Y + food.y * CELL_SIZE + CELL_SIZE / 2;

    // Apple shape
    display.fillCircle(px, py + 2, CELL_SIZE / 3, GxEPD_BLACK);
    // Stem
    display.drawLine(px, py - CELL_SIZE / 3 + 2, px + 3, py - CELL_SIZE / 3 - 3, GxEPD_BLACK);
    display.drawLine(px + 1, py - CELL_SIZE / 3 + 2, px + 4, py - CELL_SIZE / 3 - 3, GxEPD_BLACK);
}

void Snake::drawStatus() {
    // Header
    display.fillRect(0, 0, DISPLAY_WIDTH, 48, GxEPD_BLACK);

    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(20, 35);
    display.print("SNAKE");

    // Score
    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(200, 32);
    display.print(scoreText);

    // High score
    snprintf(scoreText, sizeof(scoreText), "Best: %d", highScore);
    display.setCursor(400, 32);
    display.print(scoreText);

    // Length
    snprintf(scoreText, sizeof(scoreText), "Length: %d", snakeLength);
    display.setCursor(580, 32);
    display.print(scoreText);

    display.setTextColor(GxEPD_BLACK);

    // Controls
    drawControls();
}

void Snake::drawControls() {
    int y = DISPLAY_HEIGHT - 22;
    display.drawLine(0, y - 8, DISPLAY_WIDTH, y - 8, GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, y);
    display.print("D-PAD: Change direction");

    display.setCursor(300, y);
    display.print("A: Move forward");

    display.setCursor(520, y);
    if (autoMove) {
        display.print("B: Stop auto [ON]");
    } else {
        display.print("B: Auto-move [OFF]");
    }
}
