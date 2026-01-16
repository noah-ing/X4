/**
 * Stock Portfolio Tracker for X4
 * Simple stock/IRA portfolio monitoring
 *
 * Uses Yahoo Finance API for real-time quotes
 * Configure your portfolio in the init() method
 */

#ifndef STOCKTRACKER_H
#define STOCKTRACKER_H

#include "game.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Maximum stocks to track
#define MAX_STOCKS 10

// Stock holding info
struct StockHolding {
    char symbol[12];
    float shares;
    float costBasis;  // Total cost paid
    float currentPrice;
    float previousClose;
    float dayChange;
    float dayChangePercent;
    bool valid;
};

class StockTracker : public Game {
public:
    const char* name() const override { return "Portfolio"; }
    const char* description() const override { return "Stock & IRA portfolio tracker"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

    // Add a stock to portfolio
    void addHolding(const char* symbol, float shares, float costBasis);

    // Set WiFi credentials (call before init if not using stored creds)
    void setWiFi(const char* ssid, const char* password);

private:
    // Portfolio data
    StockHolding holdings[MAX_STOCKS];
    int numHoldings = 0;

    // Totals
    float totalValue = 0;
    float totalCost = 0;
    float totalDayChange = 0;
    float totalGainLoss = 0;

    // State
    int selectedStock = 0;
    bool refreshing = false;
    bool wifiConnected = false;
    uint32_t lastRefresh = 0;
    static constexpr uint32_t REFRESH_INTERVAL = 60000; // 1 minute

    // WiFi credentials
    char wifiSSID[64] = "";
    char wifiPassword[64] = "";

    // Fetch stock data from API
    bool fetchQuote(const char* symbol, StockHolding& holding);
    bool connectWiFi();
    void disconnectWiFi();

    // Calculate totals
    void calculateTotals();

    // Drawing
    void drawHeader();
    void drawPortfolioSummary();
    void drawHoldingsList();
    void drawHoldingDetail(int index);
    void drawRefreshStatus();
    void drawNoWiFi();
    void drawMiniChart(int x, int y, int w, int h); // Placeholder

    // Format helpers
    void formatCurrency(char* buffer, size_t size, float value);
    void formatPercent(char* buffer, size_t size, float value);
};

#endif // STOCKTRACKER_H
