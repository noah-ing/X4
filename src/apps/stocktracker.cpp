/**
 * Stock Portfolio Tracker Implementation
 */

#include "apps/stocktracker.h"
#include <string.h>

void StockTracker::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    selectedStock = 0;
    refreshing = false;
    lastRefresh = 0;

    // Clear holdings
    numHoldings = 0;
    memset(holdings, 0, sizeof(holdings));

    // ======================================================
    // CONFIGURE YOUR PORTFOLIO HERE
    // ======================================================
    // Example: Add your IRA holdings
    // addHolding("SYMBOL", shares, total_cost_paid);
    //
    // addHolding("VTI", 50.0, 10000.00);    // Vanguard Total Stock
    // addHolding("VXUS", 30.0, 5000.00);    // Vanguard Intl Stock
    // ======================================================

    // Try to connect to WiFi and fetch initial data
    if (connectWiFi()) {
        refreshing = true;
        for (int i = 0; i < numHoldings; i++) {
            fetchQuote(holdings[i].symbol, holdings[i]);
        }
        calculateTotals();
        refreshing = false;
        lastRefresh = millis();
    }
}

void StockTracker::addHolding(const char* symbol, float shares, float costBasis) {
    if (numHoldings >= MAX_STOCKS) return;

    StockHolding& h = holdings[numHoldings];
    strncpy(h.symbol, symbol, sizeof(h.symbol) - 1);
    h.symbol[sizeof(h.symbol) - 1] = '\0';
    h.shares = shares;
    h.costBasis = costBasis;
    h.currentPrice = 0;
    h.previousClose = 0;
    h.dayChange = 0;
    h.dayChangePercent = 0;
    h.valid = false;

    numHoldings++;
}

void StockTracker::setWiFi(const char* ssid, const char* password) {
    strncpy(wifiSSID, ssid, sizeof(wifiSSID) - 1);
    strncpy(wifiPassword, password, sizeof(wifiPassword) - 1);
}

bool StockTracker::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        return true;
    }

    // If no credentials set, try to use stored credentials
    if (strlen(wifiSSID) == 0) {
        WiFi.begin();  // Use stored credentials
    } else {
        WiFi.begin(wifiSSID, wifiPassword);
    }

    // Wait for connection (with timeout)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    wifiConnected = (WiFi.status() == WL_CONNECTED);
    return wifiConnected;
}

void StockTracker::disconnectWiFi() {
    WiFi.disconnect();
    wifiConnected = false;
}

bool StockTracker::fetchQuote(const char* symbol, StockHolding& holding) {
    if (!wifiConnected) return false;

    HTTPClient http;

    // Using Yahoo Finance API (unofficial)
    // Format: https://query1.finance.yahoo.com/v8/finance/chart/SYMBOL?interval=1d&range=1d
    char url[256];
    snprintf(url, sizeof(url),
             "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=1d&range=1d",
             symbol);

    http.begin(url);
    http.addHeader("User-Agent", "X4Games/1.0");

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        // Parse JSON response
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonObject result = doc["chart"]["result"][0];
            JsonObject meta = result["meta"];
            JsonObject quote = result["indicators"]["quote"][0];

            holding.currentPrice = meta["regularMarketPrice"].as<float>();
            holding.previousClose = meta["previousClose"].as<float>();
            holding.dayChange = holding.currentPrice - holding.previousClose;
            holding.dayChangePercent = (holding.dayChange / holding.previousClose) * 100.0;
            holding.valid = true;

            http.end();
            return true;
        }
    }

    http.end();
    holding.valid = false;
    return false;
}

void StockTracker::calculateTotals() {
    totalValue = 0;
    totalCost = 0;
    totalDayChange = 0;

    for (int i = 0; i < numHoldings; i++) {
        if (holdings[i].valid) {
            float holdingValue = holdings[i].shares * holdings[i].currentPrice;
            float holdingDayChange = holdings[i].shares * holdings[i].dayChange;

            totalValue += holdingValue;
            totalCost += holdings[i].costBasis;
            totalDayChange += holdingDayChange;
        }
    }

    totalGainLoss = totalValue - totalCost;
}

bool StockTracker::update() {
    // Auto-refresh every minute
    if (wifiConnected && millis() - lastRefresh > REFRESH_INTERVAL) {
        refreshing = true;
        needsRedraw = true;
        draw();
        display.partialRefresh();

        for (int i = 0; i < numHoldings; i++) {
            fetchQuote(holdings[i].symbol, holdings[i]);
        }
        calculateTotals();

        refreshing = false;
        lastRefresh = millis();
        needsRedraw = true;
    }

    return !exitRequested;
}

void StockTracker::handleInput(Button btn) {
    switch (btn) {
        case Button::UP:
            if (selectedStock > 0) {
                selectedStock--;
                needsRedraw = true;
            }
            break;

        case Button::DOWN:
            if (selectedStock < numHoldings - 1) {
                selectedStock++;
                needsRedraw = true;
            }
            break;

        case Button::CONFIRM:
            // Manual refresh
            if (wifiConnected || connectWiFi()) {
                refreshing = true;
                needsRedraw = true;
                draw();
                display.partialRefresh();

                for (int i = 0; i < numHoldings; i++) {
                    fetchQuote(holdings[i].symbol, holdings[i]);
                }
                calculateTotals();

                refreshing = false;
                lastRefresh = millis();
                needsRedraw = true;
            }
            break;

        case Button::BACK:
            exitRequested = true;
            disconnectWiFi();  // Disconnect to save memory
            break;

        default:
            break;
    }
}

void StockTracker::draw() {
    display.clear();

    if (numHoldings == 0) {
        drawHeader();
        display.setFont(&FreeSans12pt7b);
        display.drawCenteredText("No holdings configured", 200);
        display.setFont(&FreeSans9pt7b);
        display.drawCenteredText("Edit stocktracker.cpp to add your stocks", 240);
        display.drawCenteredText("addHolding(\"SYMBOL\", shares, cost);", 270);
        return;
    }

    if (!wifiConnected) {
        drawNoWiFi();
        return;
    }

    drawHeader();
    drawPortfolioSummary();
    drawHoldingsList();
    drawRefreshStatus();
}

void StockTracker::drawHeader() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);

    display.setFont(&FreeSansBold18pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(20, 35);
    display.print("PORTFOLIO TRACKER");

    display.setTextColor(GxEPD_BLACK);
}

void StockTracker::drawPortfolioSummary() {
    int y = 70;

    // Total value box
    display.fillRoundRect(20, y, 360, 80, 8, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(35, y + 20);
    display.print("Total Portfolio Value");

    char valueStr[32];
    formatCurrency(valueStr, sizeof(valueStr), totalValue);
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(35, y + 60);
    display.print(valueStr);

    display.setTextColor(GxEPD_BLACK);

    // Day change box
    display.drawRoundRect(400, y, 180, 80, 8, GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(415, y + 20);
    display.print("Today's Change");

    formatCurrency(valueStr, sizeof(valueStr), totalDayChange);
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(415, y + 50);
    display.print(valueStr);

    float dayPercent = (totalDayChange / (totalValue - totalDayChange)) * 100.0;
    formatPercent(valueStr, sizeof(valueStr), dayPercent);
    display.setFont(&FreeSans12pt7b);
    display.setCursor(415, y + 72);
    display.print(valueStr);

    // Total gain/loss box
    display.drawRoundRect(600, y, 180, 80, 8, GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(615, y + 20);
    display.print("Total Gain/Loss");

    formatCurrency(valueStr, sizeof(valueStr), totalGainLoss);
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(615, y + 50);
    display.print(valueStr);

    float totalPercent = (totalGainLoss / totalCost) * 100.0;
    formatPercent(valueStr, sizeof(valueStr), totalPercent);
    display.setFont(&FreeSans12pt7b);
    display.setCursor(615, y + 72);
    display.print(valueStr);
}

void StockTracker::drawHoldingsList() {
    int startY = 170;
    int rowHeight = 55;

    // Header row
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(30, startY);
    display.print("Symbol");
    display.setCursor(150, startY);
    display.print("Price");
    display.setCursor(280, startY);
    display.print("Change");
    display.setCursor(420, startY);
    display.print("Shares");
    display.setCursor(530, startY);
    display.print("Value");
    display.setCursor(660, startY);
    display.print("Gain/Loss");

    display.drawLine(20, startY + 10, DISPLAY_WIDTH - 20, startY + 10, GxEPD_BLACK);

    // Holdings rows
    for (int i = 0; i < numHoldings && i < 5; i++) {
        int y = startY + 30 + i * rowHeight;
        StockHolding& h = holdings[i];

        bool selected = (i == selectedStock);
        if (selected) {
            display.fillRect(20, y - 15, DISPLAY_WIDTH - 40, rowHeight - 5, GxEPD_BLACK);
            display.setTextColor(GxEPD_WHITE);
        } else {
            display.setTextColor(GxEPD_BLACK);
        }

        // Symbol
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(30, y + 10);
        display.print(h.symbol);

        if (h.valid) {
            char buf[32];

            // Price
            display.setFont(&FreeSans12pt7b);
            formatCurrency(buf, sizeof(buf), h.currentPrice);
            display.setCursor(150, y + 10);
            display.print(buf);

            // Day change
            formatCurrency(buf, sizeof(buf), h.dayChange);
            display.setCursor(280, y);
            display.print(buf);
            formatPercent(buf, sizeof(buf), h.dayChangePercent);
            display.setFont(&FreeSans9pt7b);
            display.setCursor(280, y + 20);
            display.print(buf);

            // Shares
            display.setFont(&FreeSans12pt7b);
            snprintf(buf, sizeof(buf), "%.2f", h.shares);
            display.setCursor(420, y + 10);
            display.print(buf);

            // Value
            float value = h.shares * h.currentPrice;
            formatCurrency(buf, sizeof(buf), value);
            display.setCursor(530, y + 10);
            display.print(buf);

            // Gain/Loss
            float gainLoss = value - h.costBasis;
            float gainPercent = (gainLoss / h.costBasis) * 100.0;
            formatCurrency(buf, sizeof(buf), gainLoss);
            display.setCursor(660, y);
            display.print(buf);
            formatPercent(buf, sizeof(buf), gainPercent);
            display.setFont(&FreeSans9pt7b);
            display.setCursor(660, y + 20);
            display.print(buf);
        } else {
            display.setFont(&FreeSans12pt7b);
            display.setCursor(150, y + 10);
            display.print("Loading...");
        }

        display.setTextColor(GxEPD_BLACK);
    }
}

void StockTracker::drawRefreshStatus() {
    int y = DISPLAY_HEIGHT - 25;
    display.drawLine(0, y - 10, DISPLAY_WIDTH, y - 10, GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);

    if (refreshing) {
        display.setCursor(30, y);
        display.print("Refreshing...");
    } else {
        char timeStr[32];
        uint32_t elapsed = (millis() - lastRefresh) / 1000;
        snprintf(timeStr, sizeof(timeStr), "Updated %ds ago", (int)elapsed);
        display.setCursor(30, y);
        display.print(timeStr);
    }

    display.setCursor(300, y);
    display.print("A: Refresh    UP/DOWN: Select    B: Exit");
}

void StockTracker::drawNoWiFi() {
    drawHeader();

    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("No WiFi Connection", 200);

    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("Press A to retry connection", 250);
    display.drawCenteredText("Press B to exit", 280);
}

void StockTracker::formatCurrency(char* buffer, size_t size, float value) {
    if (value >= 0) {
        snprintf(buffer, size, "$%.2f", value);
    } else {
        snprintf(buffer, size, "-$%.2f", -value);
    }
}

void StockTracker::formatPercent(char* buffer, size_t size, float value) {
    if (value >= 0) {
        snprintf(buffer, size, "+%.2f%%", value);
    } else {
        snprintf(buffer, size, "%.2f%%", value);
    }
}
