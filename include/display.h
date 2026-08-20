/**
 * Display Wrapper for Xteink X4
 * Provides convenient drawing functions for the E-Ink display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "x4_hardware.h"
#include <GxEPD2_BW.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// Display type definition for GDEQ0426T82
typedef GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> DisplayType;

class Display {
public:
    Display();

    void begin();

    // Full refresh (slower, no ghosting)
    void fullRefresh();

    // Partial refresh (faster, may have ghosting)
    void partialRefresh();

    // Clear screen
    void clear();

    // Drawing functions
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

    // Text functions
    void setFont(const GFXfont* font);
    void setTextColor(uint16_t color);
    void setTextSize(uint8_t size);
    void setCursor(int16_t x, int16_t y);
    void print(const char* text);
    void print(int value);
    void println(const char* text);
    void println(int value);

    // Centered text
    void drawCenteredText(const char* text, int16_t y);
    void drawCenteredText(const char* text, int16_t x, int16_t y, int16_t w);

    // Get text bounds
    void getTextBounds(const char* text, int16_t x, int16_t y,
                       int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);

    // Screen dimensions
    int16_t width() const { return DISPLAY_WIDTH; }
    int16_t height() const { return DISPLAY_HEIGHT; }

    // Direct access to GxEPD2 display (for advanced use)
    DisplayType& raw() { return epd; }

private:
    DisplayType epd;
    bool initialized = false;
};

extern Display display;

#endif // DISPLAY_H
