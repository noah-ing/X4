/**
 * Stock Portfolio Tracker - Enhanced Implementation
 * Full charts, sparklines, market indices, and more
 */

#include "apps/stocktracker.h"
#include <string.h>
#include <math.h>

void StockTracker::init() {
    gameOver = false;
    needsRedraw = true;
    exitRequested = false;
    viewMode = ViewMode::PORTFOLIO;
    selectedStock = 0;
    scrollOffset = 0;
    chartRange = ChartRange::MONTH_1;
    refreshing = false;
    lastRefresh = 0;

    numHoldings = 0;
    memset(holdings, 0, sizeof(holdings));
    memset(indices, 0, sizeof(indices));

    // Setup market indices
    strcpy(indices[0].symbol, "^GSPC");
    strcpy(indices[0].name, "S&P 500");
    strcpy(indices[1].symbol, "^IXIC");
    strcpy(indices[1].name, "NASDAQ");
    strcpy(indices[2].symbol, "^DJI");
    strcpy(indices[2].name, "DOW");

    // Connect and fetch data
    if (connectWiFi()) {
        refreshAll();
    }
}

void StockTracker::addHolding(const char* symbol, float shares, float costBasis) {
    if (numHoldings >= MAX_STOCKS) return;

    StockHolding& h = holdings[numHoldings];
    memset(&h, 0, sizeof(StockHolding));
    strncpy(h.symbol, symbol, sizeof(h.symbol) - 1);
    h.shares = shares;
    h.costBasis = costBasis;
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

    if (strlen(wifiSSID) == 0) {
        WiFi.begin();
    } else {
        WiFi.begin(wifiSSID, wifiPassword);
    }

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

void StockTracker::refreshAll() {
    refreshing = true;
    needsRedraw = true;
    draw();
    display.partialRefresh();

    // Fetch market indices
    for (int i = 0; i < 3; i++) {
        fetchIndex(indices[i].symbol, indices[i]);
    }

    // Fetch holdings
    for (int i = 0; i < numHoldings; i++) {
        fetchQuote(holdings[i].symbol, holdings[i]);
        fetchSparkline(holdings[i].symbol, holdings[i].sparkline, holdings[i].sparklineCount);
    }

    calculateTotals();
    refreshing = false;
    lastRefresh = millis();
    needsRedraw = true;
}

bool StockTracker::fetchQuote(const char* symbol, StockHolding& holding) {
    if (!wifiConnected) return false;

    HTTPClient http;
    char url[256];
    snprintf(url, sizeof(url),
             "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=1d&range=5d",
             symbol);

    http.begin(url);
    http.addHeader("User-Agent", "X4Games/1.0");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(8192);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonObject result = doc["chart"]["result"][0];
            JsonObject meta = result["meta"];

            holding.currentPrice = meta["regularMarketPrice"].as<float>();
            holding.previousClose = meta["previousClose"].as<float>();
            holding.dayHigh = meta["regularMarketDayHigh"].as<float>();
            holding.dayLow = meta["regularMarketDayLow"].as<float>();
            holding.weekHigh52 = meta["fiftyTwoWeekHigh"].as<float>();
            holding.weekLow52 = meta["fiftyTwoWeekLow"].as<float>();

            const char* shortName = meta["shortName"];
            if (shortName) {
                strncpy(holding.name, shortName, sizeof(holding.name) - 1);
            }

            holding.dayChange = holding.currentPrice - holding.previousClose;
            holding.dayChangePercent = (holding.dayChange / holding.previousClose) * 100.0f;
            holding.valid = true;

            http.end();
            return true;
        }
    }

    http.end();
    holding.valid = false;
    return false;
}

bool StockTracker::fetchSparkline(const char* symbol, float* data, int& count) {
    if (!wifiConnected) return false;

    HTTPClient http;
    char url[256];
    snprintf(url, sizeof(url),
             "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=1d&range=1mo",
             symbol);

    http.begin(url);
    http.addHeader("User-Agent", "X4Games/1.0");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(16384);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonArray closes = doc["chart"]["result"][0]["indicators"]["quote"][0]["close"];
            count = 0;

            for (JsonVariant v : closes) {
                if (count >= SPARKLINE_POINTS) break;
                if (!v.isNull()) {
                    data[count++] = v.as<float>();
                }
            }

            http.end();
            return count > 0;
        }
    }

    http.end();
    count = 0;
    return false;
}

bool StockTracker::fetchHistory(const char* symbol, StockHolding& holding, ChartRange range) {
    if (!wifiConnected) return false;

    HTTPClient http;
    char url[256];
    const char* rangeParam = getRangeParam(range);
    const char* interval = (range == ChartRange::DAY_1) ? "5m" :
                           (range == ChartRange::WEEK_1) ? "15m" : "1d";

    snprintf(url, sizeof(url),
             "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=%s&range=%s",
             symbol, interval, rangeParam);

    http.begin(url);
    http.addHeader("User-Agent", "X4Games/1.0");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(32768);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonObject result = doc["chart"]["result"][0];
            JsonArray timestamps = result["timestamp"];
            JsonObject quote = result["indicators"]["quote"][0];
            JsonArray opens = quote["open"];
            JsonArray highs = quote["high"];
            JsonArray lows = quote["low"];
            JsonArray closes = quote["close"];
            JsonArray volumes = quote["volume"];

            holding.historyCount = 0;
            int idx = 0;

            for (size_t i = 0; i < timestamps.size() && holding.historyCount < MAX_CHART_POINTS; i++) {
                if (!closes[i].isNull()) {
                    PricePoint& p = holding.history[holding.historyCount];
                    p.timestamp = timestamps[i].as<uint32_t>();
                    p.open = opens[i].isNull() ? closes[i].as<float>() : opens[i].as<float>();
                    p.high = highs[i].isNull() ? closes[i].as<float>() : highs[i].as<float>();
                    p.low = lows[i].isNull() ? closes[i].as<float>() : lows[i].as<float>();
                    p.close = closes[i].as<float>();
                    p.volume = volumes[i].isNull() ? 0 : volumes[i].as<uint32_t>();
                    holding.historyCount++;
                }
            }

            http.end();
            return holding.historyCount > 0;
        }
    }

    http.end();
    return false;
}

bool StockTracker::fetchIndex(const char* symbol, MarketIndex& index) {
    if (!wifiConnected) return false;

    HTTPClient http;
    char url[256];
    snprintf(url, sizeof(url),
             "https://query1.finance.yahoo.com/v8/finance/chart/%s?interval=1d&range=1mo",
             symbol);

    http.begin(url);
    http.addHeader("User-Agent", "X4Games/1.0");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(16384);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonObject result = doc["chart"]["result"][0];
            JsonObject meta = result["meta"];

            index.value = meta["regularMarketPrice"].as<float>();
            float prevClose = meta["previousClose"].as<float>();
            index.change = index.value - prevClose;
            index.changePercent = (index.change / prevClose) * 100.0f;

            // Get sparkline data
            JsonArray closes = result["indicators"]["quote"][0]["close"];
            index.sparklineCount = 0;

            for (JsonVariant v : closes) {
                if (index.sparklineCount >= SPARKLINE_POINTS) break;
                if (!v.isNull()) {
                    index.sparkline[index.sparklineCount++] = v.as<float>();
                }
            }

            index.valid = true;
            http.end();
            return true;
        }
    }

    http.end();
    index.valid = false;
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
    // Auto-refresh
    if (wifiConnected && !refreshing && millis() - lastRefresh > REFRESH_INTERVAL) {
        refreshAll();
    }

    return !exitRequested;
}

void StockTracker::handleInput(Button btn) {
    switch (viewMode) {
        case ViewMode::PORTFOLIO:
            switch (btn) {
                case Button::UP:
                    if (selectedStock > 0) {
                        selectedStock--;
                        if (selectedStock < scrollOffset) scrollOffset = selectedStock;
                    }
                    needsRedraw = true;
                    break;

                case Button::DOWN:
                    if (selectedStock < numHoldings - 1) {
                        selectedStock++;
                        if (selectedStock >= scrollOffset + 4) scrollOffset = selectedStock - 3;
                    }
                    needsRedraw = true;
                    break;

                case Button::LEFT:
                    viewMode = ViewMode::MARKET_INDICES;
                    needsRedraw = true;
                    break;

                case Button::RIGHT:
                    viewMode = ViewMode::ALLOCATION;
                    needsRedraw = true;
                    break;

                case Button::CONFIRM:
                    if (numHoldings > 0) {
                        // Fetch detailed history for selected stock
                        fetchHistory(holdings[selectedStock].symbol, holdings[selectedStock], chartRange);
                        viewMode = ViewMode::STOCK_DETAIL;
                    }
                    needsRedraw = true;
                    break;

                case Button::BACK:
                    exitRequested = true;
                    disconnectWiFi();
                    break;

                default:
                    break;
            }
            break;

        case ViewMode::STOCK_DETAIL:
            switch (btn) {
                case Button::LEFT:
                    if (chartRange != ChartRange::DAY_1) {
                        chartRange = (ChartRange)((int)chartRange - 1);
                        fetchHistory(holdings[selectedStock].symbol, holdings[selectedStock], chartRange);
                    }
                    needsRedraw = true;
                    break;

                case Button::RIGHT:
                    if (chartRange != ChartRange::YEAR_1) {
                        chartRange = (ChartRange)((int)chartRange + 1);
                        fetchHistory(holdings[selectedStock].symbol, holdings[selectedStock], chartRange);
                    }
                    needsRedraw = true;
                    break;

                case Button::UP:
                    if (selectedStock > 0) {
                        selectedStock--;
                        fetchHistory(holdings[selectedStock].symbol, holdings[selectedStock], chartRange);
                    }
                    needsRedraw = true;
                    break;

                case Button::DOWN:
                    if (selectedStock < numHoldings - 1) {
                        selectedStock++;
                        fetchHistory(holdings[selectedStock].symbol, holdings[selectedStock], chartRange);
                    }
                    needsRedraw = true;
                    break;

                case Button::CONFIRM:
                    // Refresh current stock
                    fetchQuote(holdings[selectedStock].symbol, holdings[selectedStock]);
                    fetchHistory(holdings[selectedStock].symbol, holdings[selectedStock], chartRange);
                    needsRedraw = true;
                    break;

                case Button::BACK:
                    viewMode = ViewMode::PORTFOLIO;
                    needsRedraw = true;
                    break;

                default:
                    break;
            }
            break;

        case ViewMode::MARKET_INDICES:
            switch (btn) {
                case Button::CONFIRM:
                    // Refresh indices
                    for (int i = 0; i < 3; i++) {
                        fetchIndex(indices[i].symbol, indices[i]);
                    }
                    needsRedraw = true;
                    break;

                case Button::BACK:
                case Button::RIGHT:
                    viewMode = ViewMode::PORTFOLIO;
                    needsRedraw = true;
                    break;

                default:
                    break;
            }
            break;

        case ViewMode::ALLOCATION:
            switch (btn) {
                case Button::BACK:
                case Button::LEFT:
                    viewMode = ViewMode::PORTFOLIO;
                    needsRedraw = true;
                    break;

                default:
                    break;
            }
            break;
    }
}

void StockTracker::draw() {
    display.clear();

    if (!wifiConnected && !refreshing) {
        drawNoWiFi();
        return;
    }

    if (refreshing) {
        drawLoadingOverlay();
        return;
    }

    switch (viewMode) {
        case ViewMode::PORTFOLIO:
            drawPortfolioView();
            break;
        case ViewMode::STOCK_DETAIL:
            drawStockDetailView();
            break;
        case ViewMode::MARKET_INDICES:
            drawMarketIndicesView();
            break;
        case ViewMode::ALLOCATION:
            drawAllocationView();
            break;
    }
}

// ============================================================================
// Chart Drawing Functions
// ============================================================================

void StockTracker::drawLineChart(int x, int y, int w, int h, const float* data, int count,
                                  float minVal, float maxVal, bool filled) {
    if (count < 2) return;

    float range = maxVal - minVal;
    if (range < 0.001f) range = 1.0f;

    // Draw axes
    display.drawLine(x, y, x, y + h, GxEPD_BLACK);
    display.drawLine(x, y + h, x + w, y + h, GxEPD_BLACK);

    // Draw horizontal grid lines
    for (int i = 1; i < 4; i++) {
        int gy = y + (h * i) / 4;
        for (int gx = x; gx < x + w; gx += 6) {
            display.drawPixel(gx, gy, GxEPD_BLACK);
        }
    }

    // Calculate points
    int prevPx = 0, prevPy = 0;
    for (int i = 0; i < count; i++) {
        int px = x + (i * w) / (count - 1);
        int py = y + h - (int)((data[i] - minVal) / range * h);

        if (i > 0) {
            // Draw line segment
            display.drawLine(prevPx, prevPy, px, py, GxEPD_BLACK);

            // Fill under the line (dithered)
            if (filled) {
                for (int fx = prevPx; fx <= px; fx++) {
                    float t = (float)(fx - prevPx) / (px - prevPx);
                    int lineY = prevPy + (int)(t * (py - prevPy));
                    for (int fy = lineY; fy < y + h; fy += 3) {
                        if ((fx + fy) % 3 == 0) {
                            display.drawPixel(fx, fy, GxEPD_BLACK);
                        }
                    }
                }
            }
        }

        prevPx = px;
        prevPy = py;
    }

    // Draw Y-axis labels
    display.setFont(&FreeSans9pt7b);
    char label[16];

    formatCurrency(label, sizeof(label), maxVal);
    display.setCursor(x + 5, y + 12);
    display.print(label);

    formatCurrency(label, sizeof(label), minVal);
    display.setCursor(x + 5, y + h - 3);
    display.print(label);
}

void StockTracker::drawCandlestickChart(int x, int y, int w, int h,
                                         const PricePoint* data, int count) {
    if (count < 2) return;

    // Find min/max
    float minVal = data[0].low, maxVal = data[0].high;
    for (int i = 1; i < count; i++) {
        if (data[i].low < minVal) minVal = data[i].low;
        if (data[i].high > maxVal) maxVal = data[i].high;
    }

    float range = maxVal - minVal;
    if (range < 0.001f) range = 1.0f;

    // Add padding
    minVal -= range * 0.05f;
    maxVal += range * 0.05f;
    range = maxVal - minVal;

    // Draw axes
    display.drawLine(x, y, x, y + h, GxEPD_BLACK);
    display.drawLine(x, y + h, x + w, y + h, GxEPD_BLACK);

    // Calculate candle width
    int candleWidth = max(1, (w - 20) / count - 1);
    int candleSpacing = (w - 20) / count;

    for (int i = 0; i < count; i++) {
        const PricePoint& p = data[i];
        int cx = x + 10 + i * candleSpacing;

        int highY = y + (int)((maxVal - p.high) / range * h);
        int lowY = y + (int)((maxVal - p.low) / range * h);
        int openY = y + (int)((maxVal - p.open) / range * h);
        int closeY = y + (int)((maxVal - p.close) / range * h);

        // Draw wick
        display.drawLine(cx + candleWidth/2, highY, cx + candleWidth/2, lowY, GxEPD_BLACK);

        // Draw body
        int bodyTop = min(openY, closeY);
        int bodyBottom = max(openY, closeY);
        int bodyHeight = max(1, bodyBottom - bodyTop);

        if (p.close >= p.open) {
            // Green candle (up) - hollow
            display.drawRect(cx, bodyTop, candleWidth, bodyHeight, GxEPD_BLACK);
        } else {
            // Red candle (down) - filled
            display.fillRect(cx, bodyTop, candleWidth, bodyHeight, GxEPD_BLACK);
        }
    }

    // Y-axis labels
    display.setFont(&FreeSans9pt7b);
    char label[16];
    formatCurrency(label, sizeof(label), maxVal);
    display.setCursor(x + 5, y + 12);
    display.print(label);
    formatCurrency(label, sizeof(label), minVal);
    display.setCursor(x + 5, y + h - 3);
    display.print(label);
}

void StockTracker::drawSparkline(int x, int y, int w, int h, const float* data, int count) {
    if (count < 2) return;

    float minVal, maxVal;
    findMinMax(data, count, minVal, maxVal);

    float range = maxVal - minVal;
    if (range < 0.001f) range = 1.0f;

    // Draw the line
    int prevPx = 0, prevPy = 0;
    for (int i = 0; i < count; i++) {
        int px = x + (i * w) / (count - 1);
        int py = y + h - (int)((data[i] - minVal) / range * h);

        if (i > 0) {
            display.drawLine(prevPx, prevPy, px, py, GxEPD_BLACK);
        }

        prevPx = px;
        prevPy = py;
    }

    // Indicate up/down with endpoint marker
    bool up = (data[count-1] >= data[0]);
    if (up) {
        display.fillCircle(prevPx, prevPy, 2, GxEPD_BLACK);
    } else {
        display.drawCircle(prevPx, prevPy, 2, GxEPD_BLACK);
    }
}

void StockTracker::drawPieChart(int cx, int cy, int radius) {
    if (numHoldings == 0 || totalValue < 0.01f) return;

    // Calculate allocations
    float angles[MAX_STOCKS];
    float startAngle = -PI / 2;  // Start at top

    for (int i = 0; i < numHoldings; i++) {
        float value = holdings[i].shares * holdings[i].currentPrice;
        angles[i] = (value / totalValue) * 2 * PI;
    }

    // Draw pie slices with different patterns
    for (int i = 0; i < numHoldings; i++) {
        float endAngle = startAngle + angles[i];

        // Draw slice
        for (float a = startAngle; a < endAngle; a += 0.02f) {
            int x1 = cx + (int)(cos(a) * radius);
            int y1 = cy + (int)(sin(a) * radius);
            display.drawLine(cx, cy, x1, y1, GxEPD_BLACK);
        }

        // Draw arc
        for (float a = startAngle; a < endAngle; a += 0.01f) {
            int x1 = cx + (int)(cos(a) * radius);
            int y1 = cy + (int)(sin(a) * radius);
            display.drawPixel(x1, y1, GxEPD_BLACK);
        }

        // Fill with different patterns based on index
        float midAngle = startAngle + angles[i] / 2;
        int fillR = radius * 2 / 3;
        for (int r = 10; r < fillR; r += 3 + i) {
            for (float a = startAngle + 0.1f; a < endAngle - 0.1f; a += 0.1f) {
                int fx = cx + (int)(cos(a) * r);
                int fy = cy + (int)(sin(a) * r);
                display.drawPixel(fx, fy, GxEPD_BLACK);
            }
        }

        // Draw label line and text
        int labelX = cx + (int)(cos(midAngle) * (radius + 20));
        int labelY = cy + (int)(sin(midAngle) * (radius + 20));
        int lineEndX = cx + (int)(cos(midAngle) * radius);
        int lineEndY = cy + (int)(sin(midAngle) * radius);

        display.drawLine(lineEndX, lineEndY, labelX, labelY, GxEPD_BLACK);

        display.setFont(&FreeSans9pt7b);
        char label[24];
        float pct = (angles[i] / (2 * PI)) * 100;
        snprintf(label, sizeof(label), "%s %.0f%%", holdings[i].symbol, pct);

        if (labelX > cx) {
            display.setCursor(labelX + 5, labelY + 4);
        } else {
            int16_t x1, y1;
            uint16_t tw, th;
            display.getTextBounds(label, 0, 0, &x1, &y1, &tw, &th);
            display.setCursor(labelX - tw - 5, labelY + 4);
        }
        display.print(label);

        startAngle = endAngle;
    }
}

void StockTracker::drawBarChart(int x, int y, int w, int h, const float* data, int count) {
    if (count < 1) return;

    float minVal, maxVal;
    findMinMax(data, count, minVal, maxVal);

    if (minVal > 0) minVal = 0;
    float range = maxVal - minVal;
    if (range < 0.001f) range = 1.0f;

    int barWidth = (w - 10) / count - 2;
    int zeroY = y + (int)((maxVal) / range * h);

    for (int i = 0; i < count; i++) {
        int bx = x + 5 + i * (barWidth + 2);
        int barH = (int)(fabsf(data[i]) / range * h);
        int by = (data[i] >= 0) ? zeroY - barH : zeroY;

        if (data[i] >= 0) {
            display.fillRect(bx, by, barWidth, barH, GxEPD_BLACK);
        } else {
            // Negative - dithered fill
            for (int dy = 0; dy < barH; dy++) {
                for (int dx = (dy % 2); dx < barWidth; dx += 2) {
                    display.drawPixel(bx + dx, by + dy, GxEPD_BLACK);
                }
            }
        }
    }

    // Zero line
    display.drawLine(x, zeroY, x + w, zeroY, GxEPD_BLACK);
}

// ============================================================================
// View Renderers
// ============================================================================

void StockTracker::drawPortfolioView() {
    drawHeader();

    // Market indices bar
    int indexY = 55;
    display.drawLine(0, indexY + 45, DISPLAY_WIDTH, indexY + 45, GxEPD_BLACK);

    for (int i = 0; i < 3; i++) {
        int ix = 20 + i * 260;
        drawIndexCard(indices[i], ix, indexY, 240, 40);
    }

    // Portfolio summary
    int summaryY = 105;
    display.fillRect(20, summaryY, 360, 70, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, summaryY + 18);
    display.print("Portfolio Value");

    char buf[32];
    formatCurrency(buf, sizeof(buf), totalValue);
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(30, summaryY + 55);
    display.print(buf);

    display.setTextColor(GxEPD_BLACK);

    // Day change box
    display.drawRect(400, summaryY, 175, 70, GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(410, summaryY + 18);
    display.print("Today");
    formatCurrency(buf, sizeof(buf), totalDayChange);
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(410, summaryY + 40);
    display.print(buf);
    float dayPct = totalValue > 0 ? (totalDayChange / (totalValue - totalDayChange)) * 100 : 0;
    formatPercent(buf, sizeof(buf), dayPct);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(410, summaryY + 60);
    display.print(buf);

    // Total gain box
    display.drawRect(595, summaryY, 175, 70, GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(605, summaryY + 18);
    display.print("Total Gain");
    formatCurrency(buf, sizeof(buf), totalGainLoss);
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(605, summaryY + 40);
    display.print(buf);
    float totalPct = totalCost > 0 ? (totalGainLoss / totalCost) * 100 : 0;
    formatPercent(buf, sizeof(buf), totalPct);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(605, summaryY + 60);
    display.print(buf);

    // Holdings list
    int listY = 185;
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(30, listY);
    display.print("SYMBOL");
    display.setCursor(130, listY);
    display.print("PRICE");
    display.setCursor(230, listY);
    display.print("CHANGE");
    display.setCursor(370, listY);
    display.print("CHART");
    display.setCursor(510, listY);
    display.print("VALUE");
    display.setCursor(630, listY);
    display.print("GAIN/LOSS");

    display.drawLine(20, listY + 8, DISPLAY_WIDTH - 20, listY + 8, GxEPD_BLACK);

    // Draw visible holdings
    int rowY = listY + 20;
    int rowHeight = 55;
    int maxVisible = 4;

    for (int i = 0; i < maxVisible && (scrollOffset + i) < numHoldings; i++) {
        int idx = scrollOffset + i;
        drawHoldingRow(idx, rowY + i * rowHeight, idx == selectedStock);
    }

    // Scroll indicators
    if (scrollOffset > 0) {
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(DISPLAY_WIDTH - 30, listY);
        display.print("^");
    }
    if (scrollOffset + maxVisible < numHoldings) {
        display.setFont(&FreeSansBold12pt7b);
        display.setCursor(DISPLAY_WIDTH - 30, DISPLAY_HEIGHT - 50);
        display.print("v");
    }

    // Controls
    int ctrlY = DISPLAY_HEIGHT - 22;
    display.drawLine(0, ctrlY - 8, DISPLAY_WIDTH, ctrlY - 8, GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, ctrlY);
    display.print("<: Indices  >: Allocation  A: Details  B: Exit");
}

void StockTracker::drawHoldingRow(int index, int y, bool selected) {
    StockHolding& h = holdings[index];

    if (selected) {
        display.fillRect(20, y - 5, DISPLAY_WIDTH - 40, 50, GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
    }

    // Symbol
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(30, y + 20);
    display.print(h.symbol);

    if (h.valid) {
        char buf[32];

        // Price
        display.setFont(&FreeSans12pt7b);
        formatCurrency(buf, sizeof(buf), h.currentPrice);
        display.setCursor(130, y + 20);
        display.print(buf);

        // Change
        formatCurrency(buf, sizeof(buf), h.dayChange);
        display.setCursor(230, y + 12);
        display.print(buf);
        formatPercent(buf, sizeof(buf), h.dayChangePercent);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(230, y + 30);
        display.print(buf);

        // Sparkline
        if (h.sparklineCount > 0) {
            // Invert colors if selected
            if (selected) {
                // Draw white background for sparkline
                display.fillRect(365, y - 2, 120, 35, GxEPD_WHITE);
            }
            display.setTextColor(GxEPD_BLACK);
            drawSparkline(370, y, 110, 30, h.sparkline, h.sparklineCount);
            if (selected) display.setTextColor(GxEPD_WHITE);
        }

        // Value
        display.setFont(&FreeSans12pt7b);
        float value = h.shares * h.currentPrice;
        formatCurrency(buf, sizeof(buf), value);
        display.setCursor(510, y + 20);
        display.print(buf);

        // Gain/Loss
        float gain = value - h.costBasis;
        float gainPct = h.costBasis > 0 ? (gain / h.costBasis) * 100 : 0;
        formatCurrency(buf, sizeof(buf), gain);
        display.setCursor(630, y + 12);
        display.print(buf);
        formatPercent(buf, sizeof(buf), gainPct);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(630, y + 30);
        display.print(buf);
    } else {
        display.setFont(&FreeSans12pt7b);
        display.setCursor(130, y + 20);
        display.print("Loading...");
    }

    display.setTextColor(GxEPD_BLACK);
}

void StockTracker::drawStockDetailView() {
    StockHolding& h = holdings[selectedStock];

    // Header with symbol and name
    display.fillRect(0, 0, DISPLAY_WIDTH, 60, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold24pt7b);
    display.setCursor(20, 42);
    display.print(h.symbol);

    display.setFont(&FreeSans12pt7b);
    display.setCursor(150, 38);
    display.print(h.name);

    display.setTextColor(GxEPD_BLACK);

    // Price info
    int infoY = 75;
    char buf[32];

    display.setFont(&FreeSansBold24pt7b);
    formatCurrency(buf, sizeof(buf), h.currentPrice);
    display.setCursor(20, infoY + 30);
    display.print(buf);

    display.setFont(&FreeSansBold12pt7b);
    formatCurrency(buf, sizeof(buf), h.dayChange);
    display.setCursor(200, infoY + 15);
    display.print(buf);
    formatPercent(buf, sizeof(buf), h.dayChangePercent);
    display.setCursor(200, infoY + 35);
    display.print(buf);

    // Stock stats
    display.setFont(&FreeSans9pt7b);
    int statsX = 350;
    display.setCursor(statsX, infoY + 12);
    snprintf(buf, sizeof(buf), "Day: $%.2f - $%.2f", h.dayLow, h.dayHigh);
    display.print(buf);
    display.setCursor(statsX, infoY + 28);
    snprintf(buf, sizeof(buf), "52W: $%.2f - $%.2f", h.weekLow52, h.weekHigh52);
    display.print(buf);

    // Holdings info
    statsX = 580;
    float value = h.shares * h.currentPrice;
    float gain = value - h.costBasis;
    display.setCursor(statsX, infoY + 12);
    snprintf(buf, sizeof(buf), "Shares: %.2f", h.shares);
    display.print(buf);
    display.setCursor(statsX, infoY + 28);
    formatCurrency(buf, sizeof(buf), value);
    display.print("Value: ");
    display.print(buf);

    // Chart range selector
    drawChartRangeSelector(infoY + 50);

    // Main chart
    int chartX = 40, chartY = 145, chartW = 720, chartH = 280;
    display.drawRect(chartX - 2, chartY - 2, chartW + 4, chartH + 4, GxEPD_BLACK);

    if (h.historyCount > 0) {
        // Extract close prices for line chart
        float closes[MAX_CHART_POINTS];
        for (int i = 0; i < h.historyCount; i++) {
            closes[i] = h.history[i].close;
        }

        float minVal, maxVal;
        findMinMax(closes, h.historyCount, minVal, maxVal);

        // Add 5% padding
        float range = maxVal - minVal;
        minVal -= range * 0.05f;
        maxVal += range * 0.05f;

        drawLineChart(chartX, chartY, chartW, chartH, closes, h.historyCount, minVal, maxVal, true);
    } else {
        display.setFont(&FreeSans12pt7b);
        display.drawCenteredText("No chart data available", chartX, chartY + chartH/2, chartW);
    }

    // Controls
    int ctrlY = DISPLAY_HEIGHT - 22;
    display.drawLine(0, ctrlY - 8, DISPLAY_WIDTH, ctrlY - 8, GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, ctrlY);
    display.print("</>: Time range   ^/v: Prev/Next stock   A: Refresh   B: Back");
}

void StockTracker::drawChartRangeSelector(int y) {
    const char* labels[] = {"1D", "1W", "1M", "3M", "1Y"};
    int btnWidth = 60;
    int startX = (DISPLAY_WIDTH - 5 * btnWidth - 4 * 10) / 2;

    for (int i = 0; i < 5; i++) {
        int bx = startX + i * (btnWidth + 10);
        bool selected = ((int)chartRange == i);

        if (selected) {
            display.fillRoundRect(bx, y, btnWidth, 25, 4, GxEPD_BLACK);
            display.setTextColor(GxEPD_WHITE);
        } else {
            display.drawRoundRect(bx, y, btnWidth, 25, 4, GxEPD_BLACK);
            display.setTextColor(GxEPD_BLACK);
        }

        display.setFont(&FreeSansBold9pt7b);
        display.drawCenteredText(labels[i], bx, y + 17, btnWidth);
    }

    display.setTextColor(GxEPD_BLACK);
}

void StockTracker::drawMarketIndicesView() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("MARKET INDICES", 35);
    display.setTextColor(GxEPD_BLACK);

    // Large index cards with charts
    int cardW = 240, cardH = 180;
    int startX = (DISPLAY_WIDTH - 3 * cardW - 2 * 20) / 2;
    int cardY = 70;

    for (int i = 0; i < 3; i++) {
        int cx = startX + i * (cardW + 20);
        MarketIndex& idx = indices[i];

        display.drawRect(cx, cardY, cardW, cardH, GxEPD_BLACK);
        display.drawRect(cx + 1, cardY + 1, cardW - 2, cardH - 2, GxEPD_BLACK);

        // Name
        display.setFont(&FreeSansBold12pt7b);
        display.drawCenteredText(idx.name, cx, cardY + 25, cardW);

        if (idx.valid) {
            char buf[32];

            // Value
            display.setFont(&FreeSansBold18pt7b);
            formatLargeNumber(buf, sizeof(buf), idx.value);
            display.drawCenteredText(buf, cx, cardY + 55, cardW);

            // Change
            display.setFont(&FreeSans12pt7b);
            snprintf(buf, sizeof(buf), "%s%.2f (%.2f%%)",
                     idx.change >= 0 ? "+" : "", idx.change, idx.changePercent);
            display.drawCenteredText(buf, cx, cardY + 80, cardW);

            // Sparkline chart
            if (idx.sparklineCount > 0) {
                drawSparkline(cx + 20, cardY + 95, cardW - 40, 70, idx.sparkline, idx.sparklineCount);
            }
        } else {
            display.setFont(&FreeSans12pt7b);
            display.drawCenteredText("Loading...", cx, cardY + 90, cardW);
        }
    }

    // Market status
    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("Market data delayed ~15 minutes", 280);

    // Additional market info
    int infoY = 310;
    display.drawLine(20, infoY, DISPLAY_WIDTH - 20, infoY, GxEPD_BLACK);

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(30, infoY + 30);
    display.print("Portfolio vs S&P 500:");

    if (indices[0].valid && totalValue > 0) {
        float portfolioReturn = (totalGainLoss / totalCost) * 100;
        // Simplified comparison
        char buf[64];
        snprintf(buf, sizeof(buf), "Your return: %.1f%%", portfolioReturn);
        display.setFont(&FreeSans12pt7b);
        display.setCursor(30, infoY + 55);
        display.print(buf);
    }

    // Controls
    int ctrlY = DISPLAY_HEIGHT - 22;
    display.drawLine(0, ctrlY - 8, DISPLAY_WIDTH, ctrlY - 8, GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, ctrlY);
    display.print("A: Refresh   B/->: Back to Portfolio");
}

void StockTracker::drawAllocationView() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("PORTFOLIO ALLOCATION", 35);
    display.setTextColor(GxEPD_BLACK);

    if (numHoldings == 0 || totalValue < 0.01f) {
        display.setFont(&FreeSans12pt7b);
        display.drawCenteredText("No holdings to display", 240);
        return;
    }

    // Draw pie chart on left
    drawPieChart(200, 270, 140);

    // Draw legend/breakdown on right
    int legendX = 420, legendY = 80;
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(legendX, legendY);
    display.print("Holdings Breakdown");

    display.drawLine(legendX, legendY + 5, legendX + 340, legendY + 5, GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);
    int rowY = legendY + 25;

    for (int i = 0; i < numHoldings && i < 10; i++) {
        StockHolding& h = holdings[i];
        float value = h.shares * h.currentPrice;
        float pct = (value / totalValue) * 100;

        char line[64];
        snprintf(line, sizeof(line), "%s", h.symbol);
        display.setCursor(legendX, rowY);
        display.print(line);

        char valStr[32];
        formatCurrency(valStr, sizeof(valStr), value);
        display.setCursor(legendX + 80, rowY);
        display.print(valStr);

        snprintf(line, sizeof(line), "%.1f%%", pct);
        display.setCursor(legendX + 180, rowY);
        display.print(line);

        // Mini bar
        int barW = (int)(pct * 1.5);
        display.fillRect(legendX + 240, rowY - 10, barW, 12, GxEPD_BLACK);

        rowY += 30;
    }

    // Total
    display.drawLine(legendX, rowY - 5, legendX + 340, rowY - 5, GxEPD_BLACK);
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(legendX, rowY + 15);
    display.print("TOTAL");

    char totalStr[32];
    formatCurrency(totalStr, sizeof(totalStr), totalValue);
    display.setCursor(legendX + 80, rowY + 15);
    display.print(totalStr);
    display.setCursor(legendX + 180, rowY + 15);
    display.print("100%");

    // Controls
    int ctrlY = DISPLAY_HEIGHT - 22;
    display.drawLine(0, ctrlY - 8, DISPLAY_WIDTH, ctrlY - 8, GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(30, ctrlY);
    display.print("B/<-: Back to Portfolio");
}

void StockTracker::drawIndexCard(const MarketIndex& idx, int x, int y, int w, int h) {
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(x, y + 12);
    display.print(idx.name);

    if (idx.valid) {
        char buf[32];
        formatLargeNumber(buf, sizeof(buf), idx.value);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(x, y + 28);
        display.print(buf);

        snprintf(buf, sizeof(buf), "%s%.1f%%",
                 idx.changePercent >= 0 ? "+" : "", idx.changePercent);
        display.setCursor(x + 100, y + 28);
        display.print(buf);

        // Mini sparkline
        if (idx.sparklineCount > 0) {
            drawSparkline(x + 170, y + 5, 60, 30, idx.sparkline, idx.sparklineCount);
        }
    }
}

void StockTracker::drawHeader() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.setCursor(20, 35);
    display.print("PORTFOLIO TRACKER");

    // Last update time
    if (lastRefresh > 0) {
        uint32_t elapsed = (millis() - lastRefresh) / 1000;
        char timeStr[32];
        if (elapsed < 60) {
            snprintf(timeStr, sizeof(timeStr), "Updated %ds ago", (int)elapsed);
        } else {
            snprintf(timeStr, sizeof(timeStr), "Updated %dm ago", (int)(elapsed / 60));
        }
        display.setFont(&FreeSans9pt7b);
        display.setCursor(DISPLAY_WIDTH - 150, 35);
        display.print(timeStr);
    }

    display.setTextColor(GxEPD_BLACK);
}

void StockTracker::drawLoadingOverlay() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("PORTFOLIO TRACKER", 35);
    display.setTextColor(GxEPD_BLACK);

    int boxX = 200, boxY = 180, boxW = 400, boxH = 120;
    display.fillRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_WHITE);
    display.drawRoundRect(boxX, boxY, boxW, boxH, 10, GxEPD_BLACK);
    display.drawRoundRect(boxX + 2, boxY + 2, boxW - 4, boxH - 4, 8, GxEPD_BLACK);

    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("Loading...", boxX, boxY + 50, boxW);

    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("Fetching market data", boxX, boxY + 85, boxW);
}

void StockTracker::drawNoWiFi() {
    display.fillRect(0, 0, DISPLAY_WIDTH, 50, GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.drawCenteredText("PORTFOLIO TRACKER", 35);
    display.setTextColor(GxEPD_BLACK);

    display.setFont(&FreeSansBold24pt7b);
    display.drawCenteredText("No WiFi Connection", 200);

    display.setFont(&FreeSans12pt7b);
    display.drawCenteredText("Unable to connect to network", 250);

    display.setFont(&FreeSans9pt7b);
    display.drawCenteredText("Press A to retry or B to exit", 300);
}

// ============================================================================
// Helper Functions
// ============================================================================

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

void StockTracker::formatLargeNumber(char* buffer, size_t size, float value) {
    if (value >= 1000000) {
        snprintf(buffer, size, "%.2fM", value / 1000000);
    } else if (value >= 1000) {
        snprintf(buffer, size, "%.0f", value);
    } else {
        snprintf(buffer, size, "%.2f", value);
    }
}

const char* StockTracker::getRangeLabel(ChartRange range) {
    switch (range) {
        case ChartRange::DAY_1:   return "1 Day";
        case ChartRange::WEEK_1:  return "1 Week";
        case ChartRange::MONTH_1: return "1 Month";
        case ChartRange::MONTH_3: return "3 Months";
        case ChartRange::YEAR_1:  return "1 Year";
        default:                  return "1 Month";
    }
}

const char* StockTracker::getRangeParam(ChartRange range) {
    switch (range) {
        case ChartRange::DAY_1:   return "1d";
        case ChartRange::WEEK_1:  return "5d";
        case ChartRange::MONTH_1: return "1mo";
        case ChartRange::MONTH_3: return "3mo";
        case ChartRange::YEAR_1:  return "1y";
        default:                  return "1mo";
    }
}

void StockTracker::findMinMax(const float* data, int count, float& minVal, float& maxVal) {
    if (count < 1) {
        minVal = maxVal = 0;
        return;
    }

    minVal = maxVal = data[0];
    for (int i = 1; i < count; i++) {
        if (data[i] < minVal) minVal = data[i];
        if (data[i] > maxVal) maxVal = data[i];
    }
}
