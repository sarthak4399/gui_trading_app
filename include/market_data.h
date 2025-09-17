#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <map>

// Forward declarations
struct OHLCV;
struct StockData;
struct MarketScan;

/**
 * OHLCV data structure for candlestick data
 */
struct OHLCV {
    double open;
    double high;
    double low;
    double close;
    long long volume;
    std::chrono::system_clock::time_point timestamp;
    
    OHLCV() : open(0), high(0), low(0), close(0), volume(0) {}
    OHLCV(double o, double h, double l, double c, long long v) 
        : open(o), high(h), low(l), close(c), volume(v), 
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * Real-time stock data with live metrics
 */
struct StockData {
    std::string symbol;
    std::string name;
    double current_price;
    double previous_close;
    double change;
    double change_percent;
    long long volume;
    long long avg_volume;
    double volume_ratio;
    double market_cap;
    double day_high;
    double day_low;
    std::chrono::system_clock::time_point last_update;
    
    // Technical indicators
    double rsi_14;
    double sma_20;
    double sma_50;
    double ema_9;
    double ema_21;
    double vwap;
    double atr_14;
    double bollinger_upper;
    double bollinger_lower;
    double support_level;
    double resistance_level;
    
    // Trading metrics
    bool is_breakout;
    bool is_breakdown;
    bool volume_spike;
    bool near_support;
    bool near_resistance;
    
    StockData() : current_price(0), previous_close(0), change(0), change_percent(0),
                  volume(0), avg_volume(0), volume_ratio(1.0), market_cap(0),
                  day_high(0), day_low(0), rsi_14(50), sma_20(0), sma_50(0),
                  ema_9(0), ema_21(0), vwap(0), atr_14(0), bollinger_upper(0),
                  bollinger_lower(0), support_level(0), resistance_level(0),
                  is_breakout(false), is_breakdown(false), volume_spike(false),
                  near_support(false), near_resistance(false) {}
};

/**
 * Market scan results for different categories
 */
struct MarketScan {
    std::vector<StockData> top_gainers;
    std::vector<StockData> top_losers;
    std::vector<StockData> high_volume;
    std::vector<StockData> breakout_candidates;
    std::vector<StockData> breakdown_candidates;
    std::vector<StockData> near_support;
    std::vector<StockData> near_resistance;
    std::chrono::system_clock::time_point scan_time;
    
    MarketScan() : scan_time(std::chrono::system_clock::now()) {}
};

/**
 * Historical data container
 */
struct HistoricalData {
    std::string symbol;
    std::vector<OHLCV> candles;
    std::chrono::system_clock::time_point start_date;
    std::chrono::system_clock::time_point end_date;
    
    HistoricalData() : start_date(std::chrono::system_clock::now()),
                       end_date(std::chrono::system_clock::now()) {}
};

/**
 * Market status information
 */
enum class MarketStatus {
    PRE_MARKET,
    OPEN,
    CLOSED,
    POST_MARKET
};

struct MarketInfo {
    MarketStatus status;
    std::chrono::system_clock::time_point market_open;
    std::chrono::system_clock::time_point market_close;
    std::chrono::system_clock::time_point next_open;
    bool is_trading_day;
    std::string status_message;
    
    MarketInfo() : status(MarketStatus::CLOSED), is_trading_day(true), 
                   status_message("Market Closed") {}
};

/**
 * Utility functions for market data
 */
namespace MarketDataUtils {
    // Get Nifty 50 symbols with .NS suffix for Yahoo Finance
    std::vector<std::string> getNifty50Symbols();
    
    // Format price for display
    std::string formatPrice(double price, int decimals = 2);
    
    // Format volume for display
    std::string formatVolume(long long volume);
    
    // Format percentage for display
    std::string formatPercent(double percent, int decimals = 2);
    
    // Get market status based on current time (IST)
    MarketInfo getCurrentMarketStatus();
    
    // Check if current time is within trading hours
    bool isTradingTime();
    
    // Calculate time until market open/close
    std::chrono::minutes getTimeToMarketEvent();
}
