#pragma once

#include "market_data.h"
#include "yahoo_finance_api.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

/**
 * Data fetcher for managing live and historical market data
 */
class DataFetcher {
public:
    DataFetcher();
    ~DataFetcher();
    
    // Initialization
    bool initialize();
    void cleanup();
    
    // Data retrieval
    std::map<std::string, StockData> getCurrentStocks();
    StockData getStock(const std::string& symbol);
    HistoricalData getHistoricalData(const std::string& symbol, 
                                   const std::string& period = "1mo");
    
    // Market scan
    MarketScan getMarketScan();
    
    // Async operations
    void startLiveDataFeed();
    void stopLiveDataFeed();
    bool isLiveDataActive() const;
    
    // Callbacks
    void setDataUpdateCallback(std::function<void(const std::map<std::string, StockData>&)> callback);
    void setScanUpdateCallback(std::function<void(const MarketScan&)> callback);
    
    // Configuration
    void setUpdateInterval(int seconds);
    void setRateLimit(int requests_per_minute);
    
    // Status
    std::string getLastError() const;
    std::chrono::system_clock::time_point getLastUpdate() const;
    
private:
    std::unique_ptr<YahooFinanceAPI> api_;
    
    // Threading
    std::thread data_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> live_feed_active_;
    
    // Configuration
    int update_interval_seconds_;
    
    // Data storage
    std::map<std::string, StockData> current_data_;
    MarketScan current_scan_;
    std::chrono::system_clock::time_point last_update_;
    std::string last_error_;
    
    // Callbacks
    std::function<void(const std::map<std::string, StockData>&)> data_callback_;
    std::function<void(const MarketScan&)> scan_callback_;
    
    // Private methods
    void dataUpdateLoop();
    void fetchAllData();
    void notifyDataUpdate();
    void notifyScanUpdate();
};
