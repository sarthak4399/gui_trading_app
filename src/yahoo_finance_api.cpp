#include "yahoo_finance_api.h"
#include "technical_indicators.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <algorithm>
#include <regex>
#include <iostream>

using json = nlohmann::json;

// Static constants - Updated to working endpoints
const std::string YahooFinanceAPI::QUOTE_BASE_URL = "https://query1.finance.yahoo.com/v8/finance/chart";
const std::string YahooFinanceAPI::HISTORICAL_BASE_URL = "https://query1.finance.yahoo.com/v8/finance/chart";
const std::string YahooFinanceAPI::CHART_BASE_URL = "https://query1.finance.yahoo.com/v8/finance/chart";

// Callback for curl to write data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

YahooFinanceAPI::YahooFinanceAPI() 
    : timeout_seconds_(30), retry_count_(3), rate_limit_(100),
      last_request_(std::chrono::steady_clock::now()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

YahooFinanceAPI::~YahooFinanceAPI() {
    curl_global_cleanup();
}

std::vector<StockData> YahooFinanceAPI::getQuotes(const std::vector<std::string>& symbols) {
    if (symbols.empty()) return {};
    
    std::vector<StockData> result;
    
    try {
        // Since we can't batch request multiple symbols with the chart endpoint,
        // we need to make individual requests
        for (const auto& symbol : symbols) {
            enforceRateLimit();
            std::string url = CHART_BASE_URL + "/" + HttpUtils::urlEncode(symbol) + 
                             "?interval=1m&range=1d";
            std::string response = makeHttpRequest(url);
            auto stock_data = parseChartResponseForQuote(response, symbol);
            if (!stock_data.symbol.empty()) {
                result.push_back(stock_data);
            }
        }
    } catch (const std::exception& e) {
        last_error_ = "Error fetching quotes: " + std::string(e.what());
        std::cout << "❌ API Error: " << last_error_ << std::endl;
    }
    
    return result;
}

StockData YahooFinanceAPI::getQuote(const std::string& symbol) {
    auto quotes = getQuotes({symbol});
    return quotes.empty() ? StockData{} : quotes[0];
}

std::future<std::vector<StockData>> YahooFinanceAPI::getQuotesAsync(const std::vector<std::string>& symbols) {
    return std::async(std::launch::async, [this, symbols]() {
        return getQuotes(symbols);
    });
}

std::map<std::string, StockData> YahooFinanceAPI::getAllNifty50Data() {
    std::map<std::string, StockData> result;
    auto symbols = MarketDataUtils::getNifty50Symbols();
    
    // Split into batches of 10 to avoid URL length limits
    const size_t batch_size = 10;
    for (size_t i = 0; i < symbols.size(); i += batch_size) {
        size_t end = std::min(i + batch_size, symbols.size());
        std::vector<std::string> batch(symbols.begin() + i, symbols.begin() + end);
        
        auto batch_data = getQuotes(batch);
        for (const auto& stock : batch_data) {
            result[stock.symbol] = stock;
        }
        
        // Small delay between batches
        if (end < symbols.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    return result;
}

MarketScan YahooFinanceAPI::getMarketScan() {
    MarketScan scan;
    auto all_data = getAllNifty50Data();
    
    // Convert map to vector for sorting
    std::vector<StockData> stocks;
    for (const auto& pair : all_data) {
        stocks.push_back(pair.second);
    }
    
    // Sort for top gainers
    auto gainers = stocks;
    std::sort(gainers.begin(), gainers.end(), 
              [](const StockData& a, const StockData& b) {
                  return a.change_percent > b.change_percent;
              });
    scan.top_gainers = std::vector<StockData>(gainers.begin(), 
                                            gainers.begin() + std::min(10ul, gainers.size()));
    
    // Sort for top losers
    auto losers = stocks;
    std::sort(losers.begin(), losers.end(),
              [](const StockData& a, const StockData& b) {
                  return a.change_percent < b.change_percent;
              });
    scan.top_losers = std::vector<StockData>(losers.begin(),
                                           losers.begin() + std::min(10ul, losers.size()));
    
    // High volume stocks
    auto high_vol = stocks;
    std::sort(high_vol.begin(), high_vol.end(),
              [](const StockData& a, const StockData& b) {
                  return a.volume_ratio > b.volume_ratio;
              });
    scan.high_volume = std::vector<StockData>(high_vol.begin(),
                                            high_vol.begin() + std::min(10ul, high_vol.size()));
    
    // Breakout candidates
    for (const auto& stock : stocks) {
        if (stock.is_breakout) {
            scan.breakout_candidates.push_back(stock);
        }
        if (stock.is_breakdown) {
            scan.breakdown_candidates.push_back(stock);
        }
        if (stock.near_support) {
            scan.near_support.push_back(stock);
        }
        if (stock.near_resistance) {
            scan.near_resistance.push_back(stock);
        }
    }
    
    return scan;
}

std::string YahooFinanceAPI::makeHttpRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_seconds_);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; HFTTradingApp/1.0)");
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
        }
        
        curl_easy_cleanup(curl);
    }
    
    return response;
}

std::string YahooFinanceAPI::buildQuoteUrl(const std::vector<std::string>& symbols) {
    // Since v7/quote is not working, we'll use the chart endpoint for current quotes
    // For multiple symbols, we'll need to make individual requests
    if (symbols.empty()) return "";
    
    // Use the chart endpoint with minimal range for current data
    std::string url = QUOTE_BASE_URL + "/" + HttpUtils::urlEncode(symbols[0]) + 
                     "?interval=1m&range=1d";
    return url;
}

std::vector<StockData> YahooFinanceAPI::parseQuoteResponse(const std::string& json_str) {
    std::vector<StockData> stocks;
    
    try {
        auto json_data = json::parse(json_str);
        
        if (json_data.contains("quoteResponse") && 
            json_data["quoteResponse"].contains("result")) {
            
            for (const auto& quote : json_data["quoteResponse"]["result"]) {
                StockData stock;
                
                // Basic data
                stock.symbol = quote.value("symbol", "");
                stock.name = quote.value("shortName", stock.symbol);
                stock.current_price = quote.value("regularMarketPrice", 0.0);
                stock.previous_close = quote.value("regularMarketPreviousClose", 0.0);
                stock.change = quote.value("regularMarketChange", 0.0);
                stock.change_percent = quote.value("regularMarketChangePercent", 0.0);
                stock.volume = quote.value("regularMarketVolume", 0LL);
                stock.avg_volume = quote.value("averageDailyVolume10Day", 0LL);
                stock.market_cap = quote.value("marketCap", 0.0);
                stock.day_high = quote.value("regularMarketDayHigh", 0.0);
                stock.day_low = quote.value("regularMarketDayLow", 0.0);
                
                // Calculate derived metrics
                if (stock.avg_volume > 0) {
                    stock.volume_ratio = static_cast<double>(stock.volume) / stock.avg_volume;
                }
                
                stock.volume_spike = stock.volume_ratio > 1.5;
                stock.last_update = std::chrono::system_clock::now();
                
                // Get historical data for technical indicators
                auto historical = getHistoricalData(stock.symbol, "1mo", "1d");
                calculateTechnicalIndicators(stock, historical);
                identifyPatterns(stock);
                
                DataValidator::sanitizeStockData(stock);
                stocks.push_back(stock);
            }
        }
    } catch (const std::exception& e) {
        last_error_ = "JSON parsing error: " + std::string(e.what());
    }
    
    return stocks;
}

// New function to parse chart endpoint for quote data
StockData YahooFinanceAPI::parseChartResponseForQuote(const std::string& json_str, const std::string& symbol) {
    StockData stock;
    stock.symbol = symbol;
    
    try {
        auto json_data = json::parse(json_str);
        
        if (json_data.contains("chart") && 
            json_data["chart"].contains("result") &&
            !json_data["chart"]["result"].empty()) {
            
            auto result = json_data["chart"]["result"][0];
            auto meta = result.value("meta", json::object());
            
            // Extract basic info from meta
            stock.symbol = meta.value("symbol", symbol);
            stock.current_price = meta.value("regularMarketPrice", 0.0);
            stock.previous_close = meta.value("previousClose", 0.0);
            stock.day_high = meta.value("regularMarketDayHigh", 0.0);
            stock.day_low = meta.value("regularMarketDayLow", 0.0);
            stock.volume = meta.value("regularMarketVolume", 0LL);
            
            // Calculate change
            if (stock.previous_close > 0) {
                stock.change = stock.current_price - stock.previous_close;
                stock.change_percent = (stock.change / stock.previous_close) * 100.0;
            }
            
            // Get latest OHLCV data from indicators if available
            if (result.contains("indicators") && 
                result["indicators"].contains("quote") &&
                !result["indicators"]["quote"].empty()) {
                
                auto quote_data = result["indicators"]["quote"][0];
                auto volumes = quote_data.value("volume", json::array());
                
                if (!volumes.empty() && !volumes.back().is_null()) {
                    stock.volume = volumes.back().get<long long>();
                }
            }
            
            stock.last_update = std::chrono::system_clock::now();
            
            // Get historical data for technical indicators
            auto historical = getHistoricalData(stock.symbol, "1mo", "1d");
            calculateTechnicalIndicators(stock, historical);
            identifyPatterns(stock);
            
            DataValidator::sanitizeStockData(stock);
            
            std::cout << "✅ Fetched data for " << stock.symbol 
                      << " - Price: $" << stock.current_price 
                      << " (" << (stock.change >= 0 ? "+" : "") << stock.change_percent << "%)" << std::endl;
        }
    } catch (const std::exception& e) {
        last_error_ = "Chart parsing error for " + symbol + ": " + std::string(e.what());
        std::cout << "❌ Parse error for " << symbol << ": " << e.what() << std::endl;
    }
    
    return stock;
}

HistoricalData YahooFinanceAPI::getHistoricalData(const std::string& symbol,
                                                 const std::string& period,
                                                 const std::string& interval) {
    HistoricalData data;
    data.symbol = symbol;
    
    try {
        enforceRateLimit();
        std::string url = buildHistoricalUrl(symbol, period, interval);
        std::string response = makeHttpRequest(url);
        data = parseHistoricalResponse(response);
    } catch (const std::exception& e) {
        last_error_ = "Error fetching historical data: " + std::string(e.what());
    }
    
    return data;
}

std::string YahooFinanceAPI::buildHistoricalUrl(const std::string& symbol,
                                               const std::string& period,
                                               const std::string& interval) {
    return CHART_BASE_URL + "/" + HttpUtils::urlEncode(symbol) +
           "?period1=0&period2=9999999999&interval=" + interval;
}

HistoricalData YahooFinanceAPI::parseHistoricalResponse(const std::string& json_str) {
    HistoricalData data;
    
    try {
        auto json_data = json::parse(json_str);
        
        if (json_data.contains("chart") && 
            json_data["chart"].contains("result") &&
            !json_data["chart"]["result"].empty()) {
            
            auto result = json_data["chart"]["result"][0];
            data.symbol = result.value("meta", json::object()).value("symbol", "");
            
            if (result.contains("timestamp") && result.contains("indicators") &&
                result["indicators"].contains("quote") &&
                !result["indicators"]["quote"].empty()) {
                
                auto timestamps = result["timestamp"];
                auto quote = result["indicators"]["quote"][0];
                
                auto opens = quote.value("open", json::array());
                auto highs = quote.value("high", json::array());
                auto lows = quote.value("low", json::array());
                auto closes = quote.value("close", json::array());
                auto volumes = quote.value("volume", json::array());
                
                for (size_t i = 0; i < timestamps.size(); ++i) {
                    if (i < opens.size() && i < highs.size() && 
                        i < lows.size() && i < closes.size() && i < volumes.size()) {
                        
                        OHLCV candle;
                        candle.open = opens[i].is_null() ? 0.0 : opens[i].get<double>();
                        candle.high = highs[i].is_null() ? 0.0 : highs[i].get<double>();
                        candle.low = lows[i].is_null() ? 0.0 : lows[i].get<double>();
                        candle.close = closes[i].is_null() ? 0.0 : closes[i].get<double>();
                        candle.volume = volumes[i].is_null() ? 0LL : volumes[i].get<long long>();
                        
                        auto timestamp_sec = timestamps[i].get<long long>();
                        candle.timestamp = std::chrono::system_clock::from_time_t(timestamp_sec);
                        
                        if (candle.open > 0 && candle.high > 0 && candle.low > 0 && candle.close > 0) {
                            data.candles.push_back(candle);
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        last_error_ = "Historical data parsing error: " + std::string(e.what());
    }
    
    return data;
}

void YahooFinanceAPI::calculateTechnicalIndicators(StockData& stock, const HistoricalData& historical) {
    if (historical.candles.size() < 20) return;
    
    TechnicalIndicators calc;
    
    // Calculate RSI
    stock.rsi_14 = calc.calculateRSI(historical.candles, 14);
    
    // Calculate moving averages
    stock.sma_20 = calc.calculateSMA(historical.candles, 20);
    stock.sma_50 = calc.calculateSMA(historical.candles, 50);
    stock.ema_9 = calc.calculateEMA(historical.candles, 9);
    stock.ema_21 = calc.calculateEMA(historical.candles, 21);
    
    // Calculate VWAP
    stock.vwap = calc.calculateVWAP(historical.candles);
    
    // Calculate ATR
    stock.atr_14 = calc.calculateATR(historical.candles, 14);
    
    // Calculate Bollinger Bands
    auto bb = calc.calculateBollingerBands(historical.candles, 20, 2.0);
    stock.bollinger_upper = bb.upper;
    stock.bollinger_lower = bb.lower;
    
    // Calculate support and resistance
    stock.support_level = calc.findSupport(historical.candles, 20);
    stock.resistance_level = calc.findResistance(historical.candles, 20);
}

void YahooFinanceAPI::identifyPatterns(StockData& stock) {
    // Breakout detection
    stock.is_breakout = (stock.current_price > stock.resistance_level * 1.001) && 
                        stock.volume_spike;
    
    // Breakdown detection  
    stock.is_breakdown = (stock.current_price < stock.support_level * 0.999) && 
                         stock.volume_spike;
    
    // Near support/resistance
    stock.near_support = std::abs(stock.current_price - stock.support_level) / 
                         stock.support_level < 0.02;
    stock.near_resistance = std::abs(stock.current_price - stock.resistance_level) / 
                            stock.resistance_level < 0.02;
}

void YahooFinanceAPI::enforceRateLimit() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_request_);
    auto min_interval = std::chrono::milliseconds(60000 / rate_limit_); // Convert rate to interval
    
    if (elapsed < min_interval) {
        std::this_thread::sleep_for(min_interval - elapsed);
    }
    
    last_request_ = std::chrono::steady_clock::now();
}

void YahooFinanceAPI::setTimeout(int timeout_seconds) {
    timeout_seconds_ = timeout_seconds;
}

void YahooFinanceAPI::setRetryCount(int retries) {
    retry_count_ = retries;
}

void YahooFinanceAPI::setRateLimit(int requests_per_minute) {
    rate_limit_ = requests_per_minute;
}

bool YahooFinanceAPI::isConnected() const {
    // Simple connectivity test
    return last_error_.empty();
}

std::string YahooFinanceAPI::getLastError() const {
    return last_error_;
}

// HTTP Utils implementation
namespace HttpUtils {
    std::string urlEncode(const std::string& str) {
        CURL* curl = curl_easy_init();
        if (curl) {
            char* encoded = curl_easy_escape(curl, str.c_str(), str.length());
            std::string result(encoded);
            curl_free(encoded);
            curl_easy_cleanup(curl);
            return result;
        }
        return str;
    }
    
    bool isValidJson(const std::string& json) {
        try {
            auto parsed = json::parse(json);
            return true;
        } catch (...) {
            return false;
        }
    }
}

// Data Validator implementation
namespace DataValidator {
    bool isValidPrice(double price) {
        return price > 0 && price < 1000000 && std::isfinite(price);
    }
    
    bool isValidVolume(long long volume) {
        return volume >= 0 && volume < LLONG_MAX;
    }
    
    bool isValidSymbol(const std::string& symbol) {
        return !symbol.empty() && symbol.length() < 50;
    }
    
    void sanitizeStockData(StockData& data) {
        if (!isValidPrice(data.current_price)) data.current_price = 0;
        if (!isValidPrice(data.previous_close)) data.previous_close = 0;
        if (!isValidVolume(data.volume)) data.volume = 0;
        if (!isValidSymbol(data.symbol)) data.symbol = "UNKNOWN";
        
        // Ensure percentages are reasonable
        if (std::abs(data.change_percent) > 100) data.change_percent = 0;
        if (data.rsi_14 < 0 || data.rsi_14 > 100) data.rsi_14 = 50;
    }
}
