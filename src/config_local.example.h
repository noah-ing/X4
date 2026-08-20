#pragma once

#include "apps/stocktracker.h"

// Copy this file to config_local.h, which is ignored by Git, then add only the
// values you want compiled into your local firmware.
inline void configureLocalPortfolio(StockTracker& tracker) {
    // tracker.setWiFi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    // tracker.addHolding("VTI", 1.0F, 100.0F);  // symbol, shares, cost basis
}
