/**
 * X4 Hardware Abstraction Layer
 * Pin definitions and hardware constants for the Xteink X4
 *
 * This header defines the display, button, SD-card, and battery-monitor pins.
 * Confirm the pinout against the exact hardware revision before flashing.
 */

#ifndef X4_HARDWARE_H
#define X4_HARDWARE_H

#include <Arduino.h>

// ============================================================================
// Display Configuration (GxEPD2)
// ============================================================================

// Display: GDEQ0426T82 (4.26" 800x480 E-Ink)
#define DISPLAY_WIDTH   800
#define DISPLAY_HEIGHT  480

// SPI Pins for E-Ink Display
#define EPD_SCLK    8
#define EPD_MOSI    10
#define EPD_CS      21
#define EPD_DC      4
#define EPD_RST     5
#define EPD_BUSY    6

// Display rotation (270 degrees for landscape)
#define DISPLAY_ROTATION 3

// ============================================================================
// Button Configuration (ADC-based resistor ladder)
// ============================================================================

// ADC Pins
#define BTN_ADC1_PIN    1   // Back, Confirm, Left, Right
#define BTN_ADC2_PIN    2   // Volume Up, Volume Down
#define BTN_POWER_PIN   3   // Power button (digital)

// ADC thresholds for button detection (with tolerance)
// GPIO1 buttons
#define BTN_RIGHT_MIN       0
#define BTN_RIGHT_MAX       200
#define BTN_LEFT_MIN        1200
#define BTN_LEFT_MAX        1700
#define BTN_CONFIRM_MIN     2400
#define BTN_CONFIRM_MAX     2900
#define BTN_BACK_MIN        3200
#define BTN_BACK_MAX        3700

// GPIO2 buttons
#define BTN_VOLDOWN_MIN     0
#define BTN_VOLDOWN_MAX     200
#define BTN_VOLUP_MIN       2000
#define BTN_VOLUP_MAX       2500

// ============================================================================
// SD Card Configuration
// ============================================================================

#define SD_CS       12
#define SD_MISO     7
#define SD_MOSI     10
#define SD_SCLK     8

// ============================================================================
// Battery Monitoring
// ============================================================================

#define BATTERY_ADC_PIN     0   // Voltage divider (reads half voltage)
#define USB_DETECT_PIN      20  // HIGH when USB connected

// ============================================================================
// Button Enumeration
// ============================================================================

enum class Button : uint8_t {
    NONE = 0,
    LEFT,
    RIGHT,
    UP,      // Volume Up
    DOWN,    // Volume Down
    CONFIRM, // A / Select
    BACK,    // B / Cancel
    POWER
};

// ============================================================================
// Color definitions for E-Ink
// ============================================================================

#define COLOR_BLACK     GxEPD_BLACK
#define COLOR_WHITE     GxEPD_WHITE

#endif // X4_HARDWARE_H
