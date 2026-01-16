/**
 * Display Wrapper Implementation
 */

#include "display.h"
#include <SPI.h>

// Custom SPI instance for display
SPIClass displaySPI(FSPI);

// Initialize the GxEPD2 display with custom SPI pins
GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT> createDisplay() {
    return GxEPD2_BW<GxEPD2_426_GDEQ0426T82, GxEPD2_426_GDEQ0426T82::HEIGHT>(
        GxEPD2_426_GDEQ0426T82(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
    );
}

Display display;

void Display::begin() {
    if (initialized) return;

    // Initialize SPI with custom pins
    displaySPI.begin(EPD_SCLK, -1, EPD_MOSI, EPD_CS);

    // Initialize display
    epd.epd2.selectSPI(displaySPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    epd.init(0); // 0 = no debug output

    epd.setRotation(DISPLAY_ROTATION);
    epd.setTextColor(GxEPD_BLACK);
    epd.setFont(&FreeSans12pt7b);

    initialized = true;
}

void Display::fullRefresh() {
    epd.display(false); // false = full refresh
}

void Display::partialRefresh() {
    epd.display(true); // true = partial refresh
}

void Display::clear() {
    epd.fillScreen(GxEPD_WHITE);
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color) {
    epd.drawPixel(x, y, color);
}

void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    epd.drawLine(x0, y0, x1, y1, color);
}

void Display::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    epd.drawRect(x, y, w, h, color);
}

void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    epd.fillRect(x, y, w, h, color);
}

void Display::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    epd.drawCircle(x, y, r, color);
}

void Display::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    epd.fillCircle(x, y, r, color);
}

void Display::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    epd.drawRoundRect(x, y, w, h, r, color);
}

void Display::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    epd.fillRoundRect(x, y, w, h, r, color);
}

void Display::setFont(const GFXfont* font) {
    epd.setFont(font);
}

void Display::setTextColor(uint16_t color) {
    epd.setTextColor(color);
}

void Display::setTextSize(uint8_t size) {
    epd.setTextSize(size);
}

void Display::setCursor(int16_t x, int16_t y) {
    epd.setCursor(x, y);
}

void Display::print(const char* text) {
    epd.print(text);
}

void Display::print(int value) {
    epd.print(value);
}

void Display::println(const char* text) {
    epd.println(text);
}

void Display::println(int value) {
    epd.println(value);
}

void Display::drawCenteredText(const char* text, int16_t y) {
    drawCenteredText(text, 0, y, DISPLAY_WIDTH);
}

void Display::drawCenteredText(const char* text, int16_t x, int16_t y, int16_t w) {
    int16_t x1, y1;
    uint16_t tw, th;
    epd.getTextBounds(text, 0, 0, &x1, &y1, &tw, &th);
    epd.setCursor(x + (w - tw) / 2 - x1, y);
    epd.print(text);
}

void Display::getTextBounds(const char* text, int16_t x, int16_t y,
                            int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    epd.getTextBounds(text, x, y, x1, y1, w, h);
}
