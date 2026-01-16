/**
 * Snake Game for X4
 * Classic snake game adapted for E-Ink (turn-based movement)
 */

#ifndef SNAKE_H
#define SNAKE_H

#include "game.h"

// Direction enum
enum class Direction : uint8_t {
    UP = 0,
    RIGHT,
    DOWN,
    LEFT
};

// Point structure
struct Point {
    int16_t x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

class Snake : public Game {
public:
    const char* name() const override { return "Snake"; }
    const char* description() const override { return "Eat food, grow longer, don't crash!"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

private:
    // Game constants
    static constexpr int GRID_WIDTH = 26;
    static constexpr int GRID_HEIGHT = 14;
    static constexpr int CELL_SIZE = 28;
    static constexpr int MAX_SNAKE_LENGTH = GRID_WIDTH * GRID_HEIGHT;

    // Board offset for centering
    static constexpr int BOARD_OFFSET_X = (DISPLAY_WIDTH - GRID_WIDTH * CELL_SIZE) / 2;
    static constexpr int BOARD_OFFSET_Y = 55;

    // Snake body
    Point snakeBody[MAX_SNAKE_LENGTH];
    int snakeLength = 3;

    // Game state
    Direction direction = Direction::RIGHT;
    Direction nextDirection = Direction::RIGHT;
    Point food;
    int score = 0;
    int highScore = 0;
    bool autoMove = false;
    uint32_t lastMoveTime = 0;
    static constexpr uint32_t AUTO_MOVE_INTERVAL = 300; // ms between auto moves

    // Initialize snake position
    void resetSnake();

    // Spawn food at random location
    void spawnFood();

    // Move snake one step
    void moveSnake();

    // Check if point is part of snake
    bool isSnakeBody(int x, int y, bool includeHead = true) const;

    // Check for collisions
    bool checkCollision(const Point& head) const;

    // Drawing
    void drawGrid();
    void drawSnake();
    void drawFood();
    void drawStatus();
    void drawControls();
};

#endif // SNAKE_H
