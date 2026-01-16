/**
 * Input Handler for Xteink X4
 * Handles button reading via ADC with debouncing
 */

#ifndef INPUT_H
#define INPUT_H

#include "x4_hardware.h"

class Input {
public:
    void begin();
    void update();

    // Check if button was just pressed (edge detection)
    bool pressed(Button btn) const;

    // Check if button is currently held
    bool held(Button btn) const;

    // Get the last pressed button
    Button getPressed() const;

    // Wait for any button press (blocking)
    Button waitForButton();

    // Wait for specific button (blocking)
    void waitFor(Button btn);

private:
    static constexpr uint32_t DEBOUNCE_MS = 50;
    static constexpr uint32_t REPEAT_DELAY_MS = 400;
    static constexpr uint32_t REPEAT_RATE_MS = 100;

    uint8_t currentState = 0;
    uint8_t previousState = 0;
    uint8_t pressedFlags = 0;
    uint32_t lastChangeTime = 0;
    uint32_t lastRepeatTime = 0;

    Button readButtons();
    uint8_t buttonToBit(Button btn) const;
};

extern Input input;

#endif // INPUT_H
