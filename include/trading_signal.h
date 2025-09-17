#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <map>

/**
 * Trading signal types
 */
enum class SignalType {
    BUY,
    SELL,
    STRONG_BUY,
    STRONG_SELL,
    HOLD,
    NEUTRAL
};

/**
 * Signal strength levels
 */
enum class SignalStrength {
    WEAK,
    MODERATE,
    STRONG,
    VERY_STRONG
};

/**
 * Strategy types that generate signals
 */
enum class StrategyType {
    ORB,              // Opening Range Breakout
    VWAP,             // Volume Weighted Average Price
    MA_CROSSOVER,     // Moving Average Crossover
    RSI,              // RSI Overbought/Oversold
    BOLLINGER,        // Bollinger Band Squeeze
    BREAKOUT,         // Price Breakout
    SUPPORT_RESISTANCE, // Support/Resistance levels
    MOMENTUM,         // Momentum based
    VOLUME_SPIKE,     // Volume based signals
    REVERSAL          // Reversal patterns
};

/**
 * Individual trading signal
 */
struct TradingSignal {
    std::string symbol;
    SignalType type;
    SignalStrength strength;
    StrategyType strategy;
    
    double entry_price;
    double stop_loss;
    double target_1;
    double target_2;
    double confidence;
    
    std::string setup_description;
    std::string entry_reason;
    std::string risk_reward_info;
    
    std::chrono::system_clock::time_point timestamp;
    std::chrono::system_clock::time_point expiry;
    
    // Additional metrics
    double volume_confirmation;
    double technical_score;
    bool breakout_confirmed;
    bool volume_above_average;
    
    TradingSignal() : type(SignalType::NEUTRAL), strength(SignalStrength::WEAK),
                     strategy(StrategyType::ORB), entry_price(0), stop_loss(0),
                     target_1(0), target_2(0), confidence(0), 
                     timestamp(std::chrono::system_clock::now()),
                     volume_confirmation(0), technical_score(0), 
                     breakout_confirmed(false), volume_above_average(false) {}
};

/**
 * Portfolio of trading signals
 */
struct SignalPortfolio {
    std::vector<TradingSignal> active_signals;
    std::vector<TradingSignal> expired_signals;
    std::vector<TradingSignal> executed_signals;
    
    std::chrono::system_clock::time_point last_update;
    
    SignalPortfolio() : last_update(std::chrono::system_clock::now()) {}
    
    void addSignal(const TradingSignal& signal);
    void removeExpiredSignals();
    std::vector<TradingSignal> getSignalsByType(SignalType type) const;
    std::vector<TradingSignal> getSignalsByStrategy(StrategyType strategy) const;
    std::vector<TradingSignal> getHighConfidenceSignals(double min_confidence = 0.7) const;
};

/**
 * Signal performance tracking
 */
struct SignalPerformance {
    StrategyType strategy;
    int total_signals;
    int successful_signals;
    double win_rate;
    double average_return;
    double max_return;
    double max_loss;
    double average_holding_time_minutes;
    
    SignalPerformance() : strategy(StrategyType::ORB), total_signals(0),
                         successful_signals(0), win_rate(0), average_return(0),
                         max_return(0), max_loss(0), average_holding_time_minutes(0) {}
};

/**
 * Live trading setup recommendation
 */
struct TradingSetup {
    std::string symbol;
    std::string setup_name;
    SignalType primary_signal;
    std::vector<TradingSignal> supporting_signals;
    
    // Entry details
    double recommended_entry;
    double stop_loss;
    double target_1;
    double target_2;
    double risk_amount;
    double potential_reward;
    double risk_reward_ratio;
    
    // Timing
    std::string best_entry_time;
    int setup_validity_minutes;
    
    // Confidence metrics
    double overall_confidence;
    int technical_confluence;
    bool volume_confirmation;
    bool trend_alignment;
    
    std::chrono::system_clock::time_point created_at;
    
    TradingSetup() : primary_signal(SignalType::NEUTRAL), recommended_entry(0),
                    stop_loss(0), target_1(0), target_2(0), risk_amount(0),
                    potential_reward(0), risk_reward_ratio(0), setup_validity_minutes(30),
                    overall_confidence(0), technical_confluence(0),
                    volume_confirmation(false), trend_alignment(false),
                    created_at(std::chrono::system_clock::now()) {}
};

/**
 * Daily trading summary
 */
struct DailyTradingSummary {
    std::string date;
    int total_setups_generated;
    int high_confidence_setups;
    int signals_above_threshold;
    
    std::vector<TradingSetup> best_setups;
    std::vector<TradingSetup> missed_opportunities;
    
    std::map<StrategyType, int> strategy_signal_count;
    std::map<std::string, int> symbol_activity;
    
    // Market summary
    std::vector<std::string> top_performers;
    std::vector<std::string> worst_performers;
    std::vector<std::string> high_volume_stocks;
    
    double market_sentiment_score;
    std::string market_bias;
    
    DailyTradingSummary() : total_setups_generated(0), high_confidence_setups(0),
                           signals_above_threshold(0), market_sentiment_score(0) {}
};

/**
 * Utility functions for trading signals
 */
namespace SignalUtils {
    std::string signalTypeToString(SignalType type);
    std::string signalStrengthToString(SignalStrength strength);
    std::string strategyTypeToString(StrategyType strategy);
    
    SignalType stringToSignalType(const std::string& str);
    SignalStrength stringToSignalStrength(const std::string& str);
    StrategyType stringToStrategyType(const std::string& str);
    
    // Risk management
    double calculatePositionSize(double account_size, double risk_percentage, 
                               double entry_price, double stop_loss);
    double calculateRiskRewardRatio(double entry, double stop_loss, double target);
    
    // Signal validation
    bool isValidSignal(const TradingSignal& signal);
    double calculateSignalScore(const TradingSignal& signal);
    
    // Time utilities
    bool isSignalActive(const TradingSignal& signal);
    bool isWithinTradingHours(const std::chrono::system_clock::time_point& time);
    int getMinutesUntilExpiry(const TradingSignal& signal);
}
