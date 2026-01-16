/**
 * Input Handler Implementation
 */

#include "input.h"

Input input;

void Input::begin() {
    pinMode(BTN_ADC1_PIN, INPUT);
    pinMode(BTN_ADC2_PIN, INPUT);
    pinMode(BTN_POWER_PIN, INPUT_PULLUP);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    currentState = 0;
    previousState = 0;
    pressedFlags = 0;
}

void Input::update() {
    previousState = currentState;
    pressedFlags = 0;

    uint32_t now = millis();

    // Read current button state
    Button btn = readButtons();
    uint8_t newState = buttonToBit(btn);

    // Debouncing
    if (newState != currentState) {
        if (now - lastChangeTime >= DEBOUNCE_MS) {
            currentState = newState;
            lastChangeTime = now;

            // Set pressed flag for newly pressed buttons
            pressedFlags = currentState & ~previousState;
            lastRepeatTime = now;
        }
    } else if (currentState != 0 && now - lastRepeatTime >= REPEAT_DELAY_MS) {
        // Key repeat for held buttons
        if (now - lastRepeatTime >= REPEAT_RATE_MS) {
            pressedFlags = currentState;
            lastRepeatTime = now;
        }
    }
}

bool Input::pressed(Button btn) const {
    return (pressedFlags & buttonToBit(btn)) != 0;
}

bool Input::held(Button btn) const {
    return (currentState & buttonToBit(btn)) != 0;
}

Button Input::getPressed() const {
    if (pressedFlags & buttonToBit(Button::LEFT)) return Button::LEFT;
    if (pressedFlags & buttonToBit(Button::RIGHT)) return Button::RIGHT;
    if (pressedFlags & buttonToBit(Button::UP)) return Button::UP;
    if (pressedFlags & buttonToBit(Button::DOWN)) return Button::DOWN;
    if (pressedFlags & buttonToBit(Button::CONFIRM)) return Button::CONFIRM;
    if (pressedFlags & buttonToBit(Button::BACK)) return Button::BACK;
    if (pressedFlags & buttonToBit(Button::POWER)) return Button::POWER;
    return Button::NONE;
}

Button Input::waitForButton() {
    while (true) {
        update();
        Button btn = getPressed();
        if (btn != Button::NONE) {
            return btn;
        }
        delay(10);
    }
}

void Input::waitFor(Button btn) {
    while (true) {
        update();
        if (pressed(btn)) {
            return;
        }
        delay(10);
    }
}

Button Input::readButtons() {
    // Read ADC1 (Left, Right, Confirm, Back)
    int adc1 = analogRead(BTN_ADC1_PIN);

    if (adc1 >= BTN_RIGHT_MIN && adc1 <= BTN_RIGHT_MAX) {
        return Button::RIGHT;
    }
    if (adc1 >= BTN_LEFT_MIN && adc1 <= BTN_LEFT_MAX) {
        return Button::LEFT;
    }
    if (adc1 >= BTN_CONFIRM_MIN && adc1 <= BTN_CONFIRM_MAX) {
        return Button::CONFIRM;
    }
    if (adc1 >= BTN_BACK_MIN && adc1 <= BTN_BACK_MAX) {
        return Button::BACK;
    }

    // Read ADC2 (Volume Up, Volume Down)
    int adc2 = analogRead(BTN_ADC2_PIN);

    if (adc2 >= BTN_VOLDOWN_MIN && adc2 <= BTN_VOLDOWN_MAX) {
        return Button::DOWN;
    }
    if (adc2 >= BTN_VOLUP_MIN && adc2 <= BTN_VOLUP_MAX) {
        return Button::UP;
    }

    // Read Power button (digital, active LOW)
    if (digitalRead(BTN_POWER_PIN) == LOW) {
        return Button::POWER;
    }

    return Button::NONE;
}

uint8_t Input::buttonToBit(Button btn) const {
    switch (btn) {
        case Button::LEFT:    return 0x01;
        case Button::RIGHT:   return 0x02;
        case Button::UP:      return 0x04;
        case Button::DOWN:    return 0x08;
        case Button::CONFIRM: return 0x10;
        case Button::BACK:    return 0x20;
        case Button::POWER:   return 0x40;
        default:              return 0x00;
    }
}
