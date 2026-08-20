/**
 * Experimental local holdings display for X4
 * Charts and quotes are fetched from an unofficial Yahoo Finance endpoint.
 *
 * Features:
 * - Delayed or near-real-time quotes when the upstream endpoint is available
 * - Historical price charts (1D, 1W, 1M, 3M, 1Y)
 * - Sparkline mini-charts in list view
 * - Market indices (S&P 500, NASDAQ, DOW)
 * - Portfolio allocation pie chart
 * - Detailed stock view with full chart
 */

#ifndef STOCKTRACKER_H
#define STOCKTRACKER_H

#include "game.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configuration
#define MAX_STOCKS 15
#define MAX_CHART_POINTS 60
#define SPARKLINE_POINTS 20

// Chart time ranges
enum class ChartRange : uint8_t {
    DAY_1 = 0,
    WEEK_1,
    MONTH_1,
    MONTH_3,
    YEAR_1,
    RANGE_COUNT
};

// View modes
enum class ViewMode : uint8_t {
    PORTFOLIO,      // Main portfolio view
    STOCK_DETAIL,   // Single stock with chart
    MARKET_INDICES, // Major indices
    ALLOCATION      // Pie chart of holdings
};

// Historical price point
struct PricePoint {
    uint32_t timestamp;
    float open;
    float high;
    float low;
    float close;
    uint32_t volume;
};

// Stock holding with history
struct StockHolding {
    char symbol[12];
    char name[32];
    float shares;
    float costBasis;
    float currentPrice;
    float previousClose;
    float dayHigh;
    float dayLow;
    float weekHigh52;
    float weekLow52;
    float dayChange;
    float dayChangePercent;

    // Sparkline data (last 20 points)
    float sparkline[SPARKLINE_POINTS];
    int sparklineCount;

    // Full chart data
    PricePoint history[MAX_CHART_POINTS];
    int historyCount;

    bool valid;
};

// Market index data
struct MarketIndex {
    char symbol[12];
    char name[24];
    float value;
    float change;
    float changePercent;
    float sparkline[SPARKLINE_POINTS];
    int sparklineCount;
    bool valid;
};

class StockTracker : public Game {
public:
    const char* name() const override { return "Portfolio"; }
    const char* description() const override { return "Stock tracker with charts"; }

    void init() override;
    bool update() override;
    void draw() override;
    void handleInput(Button btn) override;

    // Portfolio management
    void addHolding(const char* symbol, float shares, float costBasis);
    void setWiFi(const char* ssid, const char* password);

private:
    // Portfolio data
    StockHolding holdings[MAX_STOCKS];
    int numHoldings = 0;

    // Market indices
    MarketIndex indices[3];  // S&P 500, NASDAQ, DOW

    // Totals
    float totalValue = 0;
    float totalCost = 0;
    float totalDayChange = 0;
    float totalGainLoss = 0;

    // UI State
    ViewMode viewMode = ViewMode::PORTFOLIO;
    int selectedStock = 0;
    int scrollOffset = 0;
    ChartRange chartRange = ChartRange::MONTH_1;
    bool refreshing = false;
    bool wifiConnected = false;
    uint32_t lastRefresh = 0;
    static constexpr uint32_t REFRESH_INTERVAL = 60000;

    // WiFi
    char wifiSSID[64] = "";
    char wifiPassword[64] = "";

    // API functions
    bool connectWiFi();
    bool synchronizeClock();
    void disconnectWiFi();
    bool fetchQuote(const char* symbol, StockHolding& holding);
    bool fetchHistory(const char* symbol, StockHolding& holding, ChartRange range);
    bool fetchSparkline(const char* symbol, float* data, int& count);
    bool fetchIndex(const char* symbol, MarketIndex& index);
    void refreshAll();
    void calculateTotals();

    // Chart rendering
    void drawLineChart(int x, int y, int w, int h, const float* data, int count,
                       float minVal, float maxVal, bool filled = false);
    void drawCandlestickChart(int x, int y, int w, int h,
                              const PricePoint* data, int count);
    void drawSparkline(int x, int y, int w, int h, const float* data, int count);
    void drawPieChart(int cx, int cy, int radius);
    void drawBarChart(int x, int y, int w, int h, const float* data, int count);

    // View renderers
    void drawPortfolioView();
    void drawStockDetailView();
    void drawMarketIndicesView();
    void drawAllocationView();

    // UI components
    void drawHeader();
    void drawHoldingRow(int index, int y, bool selected);
    void drawIndexCard(const MarketIndex& idx, int x, int y, int w, int h);
    void drawChartRangeSelector(int y);
    void drawLoadingOverlay();
    void drawNoWiFi();

    // Helpers
    void formatCurrency(char* buffer, size_t size, float value);
    void formatPercent(char* buffer, size_t size, float value);
    void formatLargeNumber(char* buffer, size_t size, float value);
    const char* getRangeLabel(ChartRange range);
    const char* getRangeParam(ChartRange range);
    void findMinMax(const float* data, int count, float& minVal, float& maxVal);
};

#endif // STOCKTRACKER_H
