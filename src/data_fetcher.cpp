#include "data_fetcher.h"
#include <thread>
#include <chrono>
#include <iostream>

DataFetcher::DataFetcher() : running_(false), live_feed_active_(false), 
                           update_interval_seconds_(5) {
    api_ = std::make_unique<YahooFinanceAPI>();
}

DataFetcher::~DataFetcher() {
    cleanup();
}

bool DataFetcher::initialize() {
    try {
        // Test API connection
        auto test_quote = api_->getQuote("RELIANCE.NS");
        if (test_quote.symbol.empty()) {
            last_error_ = "Failed to connect to Yahoo Finance API";
            return false;
        }
        
        std::cout << "✅ Data fetcher initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        last_error_ = "DataFetcher initialization failed: " + std::string(e.what());
        return false;
    }
}

void DataFetcher::cleanup() {
    stopLiveDataFeed();
}

std::map<std::string, StockData> DataFetcher::getCurrentStocks() {
    return current_data_;
}

StockData DataFetcher::getStock(const std::string& symbol) {
    auto it = current_data_.find(symbol);
    return (it != current_data_.end()) ? it->second : StockData{};
}

HistoricalData DataFetcher::getHistoricalData(const std::string& symbol, const std::string& period) {
    return api_->getHistoricalData(symbol, period);
}

MarketScan DataFetcher::getMarketScan() {
    return current_scan_;
}

void DataFetcher::startLiveDataFeed() {
    if (live_feed_active_) return;
    
    running_ = true;
    live_feed_active_ = true;
    data_thread_ = std::thread(&DataFetcher::dataUpdateLoop, this);
    
    std::cout << "📡 Live data feed started" << std::endl;
}

void DataFetcher::stopLiveDataFeed() {
    running_ = false;
    live_feed_active_ = false;
    
    if (data_thread_.joinable()) {
        data_thread_.join();
    }
    
    std::cout << "⏹️  Live data feed stopped" << std::endl;
}

bool DataFetcher::isLiveDataActive() const {
    return live_feed_active_;
}

void DataFetcher::setDataUpdateCallback(std::function<void(const std::map<std::string, StockData>&)> callback) {
    data_callback_ = callback;
}

void DataFetcher::setScanUpdateCallback(std::function<void(const MarketScan&)> callback) {
    scan_callback_ = callback;
}

void DataFetcher::setUpdateInterval(int seconds) {
    update_interval_seconds_ = seconds;
}

void DataFetcher::setRateLimit(int requests_per_minute) {
    api_->setRateLimit(requests_per_minute);
}

std::string DataFetcher::getLastError() const {
    return last_error_;
}

std::chrono::system_clock::time_point DataFetcher::getLastUpdate() const {
    return last_update_;
}

void DataFetcher::dataUpdateLoop() {
    while (running_) {
        try {
            fetchAllData();
            notifyDataUpdate();
            notifyScanUpdate();
            
            last_update_ = std::chrono::system_clock::now();
            
            std::this_thread::sleep_for(std::chrono::seconds(update_interval_seconds_));
            
        } catch (const std::exception& e) {
            last_error_ = "Data update error: " + std::string(e.what());
            std::cerr << "⚠️  " << last_error_ << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10)); // Wait longer on error
        }
    }
}

void DataFetcher::fetchAllData() {
    // Fetch all Nifty 50 data
    current_data_ = api_->getAllNifty50Data();
    
    // Generate market scan
    current_scan_ = api_->getMarketScan();
}

void DataFetcher::notifyDataUpdate() {
    if (data_callback_) {
        data_callback_(current_data_);
    }
}

void DataFetcher::notifyScanUpdate() {
    if (scan_callback_) {
        scan_callback_(current_scan_);
    }
}
