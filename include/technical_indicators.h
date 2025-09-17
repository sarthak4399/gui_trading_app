#pragma once

#include "market_data.h"
#include <vector>

/**
 * Bollinger Bands result structure
 */
struct BollingerBands {
    double upper;
    double middle;
    double lower;
    
    BollingerBands() : upper(0), middle(0), lower(0) {}
    BollingerBands(double u, double m, double l) : upper(u), middle(m), lower(l) {}
};

/**
 * MACD result structure
 */
struct MACD {
    double macd_line;
    double signal_line;
    double histogram;
    
    MACD() : macd_line(0), signal_line(0), histogram(0) {}
    MACD(double macd, double signal, double hist) 
        : macd_line(macd), signal_line(signal), histogram(hist) {}
};

/**
 * Technical Indicators Calculator
 */
class TechnicalIndicators {
public:
    TechnicalIndicators() = default;
    
    // Moving Averages
    static double calculateSMA(const std::vector<OHLCV>& data, int periods);
    static double calculateEMA(const std::vector<OHLCV>& data, int periods);
    static double calculateWMA(const std::vector<OHLCV>& data, int periods);
    
    // Oscillators
    static double calculateRSI(const std::vector<OHLCV>& data, int periods = 14);
    static double calculateStochastic(const std::vector<OHLCV>& data, int k_periods = 14, int d_periods = 3);
    static double calculateWilliamsR(const std::vector<OHLCV>& data, int periods = 14);
    
    // Volatility Indicators
    static double calculateATR(const std::vector<OHLCV>& data, int periods = 14);
    static BollingerBands calculateBollingerBands(const std::vector<OHLCV>& data, int periods = 20, double multiplier = 2.0);
    
    // Volume Indicators
    static double calculateVWAP(const std::vector<OHLCV>& data);
    static double calculateOBV(const std::vector<OHLCV>& data);
    
    // Trend Indicators
    static MACD calculateMACD(const std::vector<OHLCV>& data, int fast_period = 12, int slow_period = 26, int signal_period = 9);
    static double calculateADX(const std::vector<OHLCV>& data, int periods = 14);
    
    // Support and Resistance
    static double findSupport(const std::vector<OHLCV>& data, int lookback = 20);
    static double findResistance(const std::vector<OHLCV>& data, int lookback = 20);
    static std::vector<double> findPivotPoints(const std::vector<OHLCV>& data);
    
    // Pattern Recognition
    static bool isDoji(const OHLCV& candle, double threshold = 0.001);
    static bool isHammer(const OHLCV& candle);
    static bool isShootingStar(const OHLCV& candle);
    static bool isEngulfing(const OHLCV& prev, const OHLCV& current);
    
    // Utility functions
    static double calculateTrueRange(const OHLCV& current, const OHLCV& previous);
    static double calculateTypicalPrice(const OHLCV& candle);
    static std::vector<double> calculateReturns(const std::vector<OHLCV>& data);
    
private:
    // Helper functions
    static std::vector<double> extractClosePrices(const std::vector<OHLCV>& data);
    static std::vector<double> extractHighPrices(const std::vector<OHLCV>& data);
    static std::vector<double> extractLowPrices(const std::vector<OHLCV>& data);
    static double standardDeviation(const std::vector<double>& data, double mean);
};
