#pragma once

#include "market_data.h"
#include <string>
#include <vector>
#include <map>
#include <future>
#include <functional>

/**
 * Yahoo Finance API client for fetching live and historical data
 */
class YahooFinanceAPI {
public:
    YahooFinanceAPI();
    ~YahooFinanceAPI();
    
    // Sync methods
    std::vector<StockData> getQuotes(const std::vector<std::string>& symbols);
    StockData getQuote(const std::string& symbol);
    HistoricalData getHistoricalData(const std::string& symbol, 
                                    const std::string& period = "1mo",
                                    const std::string& interval = "1d");
    
    // Async methods
    std::future<std::vector<StockData>> getQuotesAsync(const std::vector<std::string>& symbols);
    std::future<StockData> getQuoteAsync(const std::string& symbol);
    std::future<HistoricalData> getHistoricalDataAsync(const std::string& symbol,
                                                       const std::string& period = "1mo",
                                                       const std::string& interval = "1d");
    
    // Batch operations
    std::map<std::string, StockData> getAllNifty50Data();
    std::future<std::map<std::string, StockData>> getAllNifty50DataAsync();
    
    // Market summary
    MarketScan getMarketScan();
    std::future<MarketScan> getMarketScanAsync();
    
    // Configuration
    void setTimeout(int timeout_seconds);
    void setRetryCount(int retries);
    void setRateLimit(int requests_per_minute);
    
    // Status
    bool isConnected() const;
    std::string getLastError() const;
    
private:
    // HTTP client methods
    std::string makeHttpRequest(const std::string& url);
    std::string buildQuoteUrl(const std::vector<std::string>& symbols);
    std::string buildHistoricalUrl(const std::string& symbol, 
                                 const std::string& period,
                                 const std::string& interval);
    
    // JSON parsing methods
    std::vector<StockData> parseQuoteResponse(const std::string& json);
    StockData parseChartResponseForQuote(const std::string& json, const std::string& symbol);
    StockData parseQuoteData(const std::string& json_object);
    HistoricalData parseHistoricalResponse(const std::string& json);
    
    // Data processing
    void calculateTechnicalIndicators(StockData& stock, const HistoricalData& historical);
    void identifyPatterns(StockData& stock);
    
    // Rate limiting
    void enforceRateLimit();
    
    // Configuration
    int timeout_seconds_;
    int retry_count_;
    int rate_limit_;
    std::chrono::steady_clock::time_point last_request_;
    
    // Error handling
    std::string last_error_;
    
    // Base URLs
    static const std::string QUOTE_BASE_URL;
    static const std::string HISTORICAL_BASE_URL;
    static const std::string CHART_BASE_URL;
};

/**
 * HTTP utility functions
 */
namespace HttpUtils {
    std::string urlEncode(const std::string& str);
    std::string httpGet(const std::string& url, int timeout_seconds = 30);
    bool isValidJson(const std::string& json);
}

/**
 * Data validation utilities
 */
namespace DataValidator {
    bool isValidPrice(double price);
    bool isValidVolume(long long volume);
    bool isValidSymbol(const std::string& symbol);
    void sanitizeStockData(StockData& data);
}
