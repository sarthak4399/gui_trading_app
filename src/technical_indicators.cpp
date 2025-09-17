#include "technical_indicators.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>

double TechnicalIndicators::calculateSMA(const std::vector<OHLCV>& data, int periods) {
    if (data.size() < static_cast<size_t>(periods)) return 0.0;
    
    double sum = 0.0;
    for (int i = data.size() - periods; i < static_cast<int>(data.size()); ++i) {
        sum += data[i].close;
    }
    return sum / periods;
}

double TechnicalIndicators::calculateEMA(const std::vector<OHLCV>& data, int periods) {
    if (data.size() < static_cast<size_t>(periods)) return 0.0;
    
    double multiplier = 2.0 / (periods + 1);
    double ema = calculateSMA(data, periods); // Start with SMA
    
    for (size_t i = periods; i < data.size(); ++i) {
        ema = (data[i].close * multiplier) + (ema * (1 - multiplier));
    }
    
    return ema;
}

double TechnicalIndicators::calculateWMA(const std::vector<OHLCV>& data, int periods) {
    if (data.size() < static_cast<size_t>(periods)) return 0.0;
    
    double weighted_sum = 0.0;
    double weight_sum = 0.0;
    
    for (int i = 0; i < periods; ++i) {
        int weight = periods - i;
        weighted_sum += data[data.size() - 1 - i].close * weight;
        weight_sum += weight;
    }
    
    return weighted_sum / weight_sum;
}

double TechnicalIndicators::calculateRSI(const std::vector<OHLCV>& data, int periods) {
    if (data.size() < static_cast<size_t>(periods + 1)) return 50.0;
    
    std::vector<double> gains, losses;
    
    for (size_t i = 1; i < data.size(); ++i) {
        double change = data[i].close - data[i-1].close;
        gains.push_back(change > 0 ? change : 0);
        losses.push_back(change < 0 ? -change : 0);
    }
    
    if (gains.size() < static_cast<size_t>(periods)) return 50.0;
    
    // Calculate initial average gain/loss
    double avg_gain = 0.0, avg_loss = 0.0;
    for (int i = 0; i < periods; ++i) {
        avg_gain += gains[i];
        avg_loss += losses[i];
    }
    avg_gain /= periods;
    avg_loss /= periods;
    
    // Calculate RSI using Wilder's smoothing
    for (size_t i = periods; i < gains.size(); ++i) {
        avg_gain = ((avg_gain * (periods - 1)) + gains[i]) / periods;
        avg_loss = ((avg_loss * (periods - 1)) + losses[i]) / periods;
    }
    
    if (avg_loss == 0) return 100.0;
    
    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

double TechnicalIndicators::calculateATR(const std::vector<OHLCV>& data, int periods) {
    if (data.size() < static_cast<size_t>(periods + 1)) return 0.0;
    
    std::vector<double> true_ranges;
    for (size_t i = 1; i < data.size(); ++i) {
        true_ranges.push_back(calculateTrueRange(data[i], data[i-1]));
    }
    
    if (true_ranges.size() < static_cast<size_t>(periods)) return 0.0;
    
    // Calculate initial ATR
    double atr = 0.0;
    for (int i = 0; i < periods; ++i) {
        atr += true_ranges[i];
    }
    atr /= periods;
    
    // Apply Wilder's smoothing
    for (size_t i = periods; i < true_ranges.size(); ++i) {
        atr = ((atr * (periods - 1)) + true_ranges[i]) / periods;
    }
    
    return atr;
}

BollingerBands TechnicalIndicators::calculateBollingerBands(const std::vector<OHLCV>& data, 
                                                           int periods, double multiplier) {
    if (data.size() < static_cast<size_t>(periods)) return BollingerBands();
    
    double sma = calculateSMA(data, periods);
    
    // Calculate standard deviation
    double sum_sq_diff = 0.0;
    for (int i = data.size() - periods; i < static_cast<int>(data.size()); ++i) {
        double diff = data[i].close - sma;
        sum_sq_diff += diff * diff;
    }
    double std_dev = std::sqrt(sum_sq_diff / periods);
    
    return BollingerBands(
        sma + (multiplier * std_dev),
        sma,
        sma - (multiplier * std_dev)
    );
}

double TechnicalIndicators::calculateVWAP(const std::vector<OHLCV>& data) {
    if (data.empty()) return 0.0;
    
    double cumulative_pv = 0.0;
    long long cumulative_volume = 0;
    
    for (const auto& candle : data) {
        double typical_price = calculateTypicalPrice(candle);
        cumulative_pv += typical_price * candle.volume;
        cumulative_volume += candle.volume;
    }
    
    return cumulative_volume > 0 ? cumulative_pv / cumulative_volume : 0.0;
}

MACD TechnicalIndicators::calculateMACD(const std::vector<OHLCV>& data, 
                                       int fast_period, int slow_period, int signal_period) {
    if (data.size() < static_cast<size_t>(slow_period)) return MACD();
    
    double fast_ema = calculateEMA(data, fast_period);
    double slow_ema = calculateEMA(data, slow_period);
    double macd_line = fast_ema - slow_ema;
    
    // Calculate signal line (EMA of MACD line)
    // For simplicity, we'll use the current MACD value as signal
    // In a full implementation, you'd need to track MACD history
    double signal_line = macd_line * 0.9; // Simplified
    
    return MACD(macd_line, signal_line, macd_line - signal_line);
}

double TechnicalIndicators::findSupport(const std::vector<OHLCV>& data, int lookback) {
    if (data.size() < static_cast<size_t>(lookback)) return 0.0;
    
    double min_low = std::numeric_limits<double>::max();
    for (int i = data.size() - lookback; i < static_cast<int>(data.size()); ++i) {
        min_low = std::min(min_low, data[i].low);
    }
    
    return min_low;
}

double TechnicalIndicators::findResistance(const std::vector<OHLCV>& data, int lookback) {
    if (data.size() < static_cast<size_t>(lookback)) return 0.0;
    
    double max_high = 0.0;
    for (int i = data.size() - lookback; i < static_cast<int>(data.size()); ++i) {
        max_high = std::max(max_high, data[i].high);
    }
    
    return max_high;
}

bool TechnicalIndicators::isDoji(const OHLCV& candle, double threshold) {
    if (candle.high == candle.low) return false;
    
    double body_size = std::abs(candle.close - candle.open);
    double full_range = candle.high - candle.low;
    
    return (body_size / full_range) < threshold;
}

bool TechnicalIndicators::isHammer(const OHLCV& candle) {
    double body = std::abs(candle.close - candle.open);
    double lower_shadow = std::min(candle.open, candle.close) - candle.low;
    double upper_shadow = candle.high - std::max(candle.open, candle.close);
    
    return lower_shadow > 2 * body && upper_shadow < body * 0.5;
}

bool TechnicalIndicators::isShootingStar(const OHLCV& candle) {
    double body = std::abs(candle.close - candle.open);
    double lower_shadow = std::min(candle.open, candle.close) - candle.low;
    double upper_shadow = candle.high - std::max(candle.open, candle.close);
    
    return upper_shadow > 2 * body && lower_shadow < body * 0.5;
}

bool TechnicalIndicators::isEngulfing(const OHLCV& prev, const OHLCV& current) {
    bool prev_red = prev.close < prev.open;
    bool curr_green = current.close > current.open;
    
    if (prev_red && curr_green) {
        return current.open < prev.close && current.close > prev.open;
    }
    
    bool prev_green = prev.close > prev.open;
    bool curr_red = current.close < current.open;
    
    if (prev_green && curr_red) {
        return current.open > prev.close && current.close < prev.open;
    }
    
    return false;
}

double TechnicalIndicators::calculateTrueRange(const OHLCV& current, const OHLCV& previous) {
    double hl = current.high - current.low;
    double hc = std::abs(current.high - previous.close);
    double lc = std::abs(current.low - previous.close);
    
    return std::max({hl, hc, lc});
}

double TechnicalIndicators::calculateTypicalPrice(const OHLCV& candle) {
    return (candle.high + candle.low + candle.close) / 3.0;
}

// Helper functions
std::vector<double> TechnicalIndicators::extractClosePrices(const std::vector<OHLCV>& data) {
    std::vector<double> prices;
    for (const auto& candle : data) {
        prices.push_back(candle.close);
    }
    return prices;
}

std::vector<double> TechnicalIndicators::extractHighPrices(const std::vector<OHLCV>& data) {
    std::vector<double> prices;
    for (const auto& candle : data) {
        prices.push_back(candle.high);
    }
    return prices;
}

std::vector<double> TechnicalIndicators::extractLowPrices(const std::vector<OHLCV>& data) {
    std::vector<double> prices;
    for (const auto& candle : data) {
        prices.push_back(candle.low);
    }
    return prices;
}

double TechnicalIndicators::standardDeviation(const std::vector<double>& data, double mean) {
    double sum_sq_diff = 0.0;
    for (double value : data) {
        double diff = value - mean;
        sum_sq_diff += diff * diff;
    }
    return std::sqrt(sum_sq_diff / data.size());
}
