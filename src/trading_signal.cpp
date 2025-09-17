#include "trading_signal.h"
#include <algorithm>
#include <cmath>
#include <sstream>

// SignalPortfolio methods
void SignalPortfolio::addSignal(const TradingSignal& signal) {
    active_signals.push_back(signal);
    last_update = std::chrono::system_clock::now();
}

void SignalPortfolio::removeExpiredSignals() {
    auto now = std::chrono::system_clock::now();
    
    auto it = std::remove_if(active_signals.begin(), active_signals.end(),
                           [now](const TradingSignal& signal) {
                               return now > signal.expiry;
                           });
    
    // Move expired signals to expired_signals vector
    for (auto exp_it = it; exp_it != active_signals.end(); ++exp_it) {
        expired_signals.push_back(*exp_it);
    }
    
    active_signals.erase(it, active_signals.end());
}

std::vector<TradingSignal> SignalPortfolio::getSignalsByType(SignalType type) const {
    std::vector<TradingSignal> result;
    std::copy_if(active_signals.begin(), active_signals.end(),
                std::back_inserter(result),
                [type](const TradingSignal& signal) {
                    return signal.type == type;
                });
    return result;
}

std::vector<TradingSignal> SignalPortfolio::getSignalsByStrategy(StrategyType strategy) const {
    std::vector<TradingSignal> result;
    std::copy_if(active_signals.begin(), active_signals.end(),
                std::back_inserter(result),
                [strategy](const TradingSignal& signal) {
                    return signal.strategy == strategy;
                });
    return result;
}

std::vector<TradingSignal> SignalPortfolio::getHighConfidenceSignals(double min_confidence) const {
    std::vector<TradingSignal> result;
    std::copy_if(active_signals.begin(), active_signals.end(),
                std::back_inserter(result),
                [min_confidence](const TradingSignal& signal) {
                    return signal.confidence >= min_confidence;
                });
    return result;
}

// SignalUtils namespace implementation
namespace SignalUtils {
    
    std::string signalTypeToString(SignalType type) {
        switch (type) {
            case SignalType::BUY: return "BUY";
            case SignalType::SELL: return "SELL";
            case SignalType::STRONG_BUY: return "STRONG BUY";
            case SignalType::STRONG_SELL: return "STRONG SELL";
            case SignalType::HOLD: return "HOLD";
            case SignalType::NEUTRAL: return "NEUTRAL";
            default: return "UNKNOWN";
        }
    }
    
    std::string signalStrengthToString(SignalStrength strength) {
        switch (strength) {
            case SignalStrength::WEAK: return "WEAK";
            case SignalStrength::MODERATE: return "MODERATE";
            case SignalStrength::STRONG: return "STRONG";
            case SignalStrength::VERY_STRONG: return "VERY STRONG";
            default: return "UNKNOWN";
        }
    }
    
    std::string strategyTypeToString(StrategyType strategy) {
        switch (strategy) {
            case StrategyType::ORB: return "ORB";
            case StrategyType::VWAP: return "VWAP";
            case StrategyType::MA_CROSSOVER: return "MA Crossover";
            case StrategyType::RSI: return "RSI";
            case StrategyType::BOLLINGER: return "Bollinger Bands";
            case StrategyType::BREAKOUT: return "Breakout";
            case StrategyType::SUPPORT_RESISTANCE: return "Support/Resistance";
            case StrategyType::MOMENTUM: return "Momentum";
            case StrategyType::VOLUME_SPIKE: return "Volume Spike";
            case StrategyType::REVERSAL: return "Reversal";
            default: return "UNKNOWN";
        }
    }
    
    SignalType stringToSignalType(const std::string& str) {
        if (str == "BUY") return SignalType::BUY;
        if (str == "SELL") return SignalType::SELL;
        if (str == "STRONG BUY") return SignalType::STRONG_BUY;
        if (str == "STRONG SELL") return SignalType::STRONG_SELL;
        if (str == "HOLD") return SignalType::HOLD;
        return SignalType::NEUTRAL;
    }
    
    SignalStrength stringToSignalStrength(const std::string& str) {
        if (str == "WEAK") return SignalStrength::WEAK;
        if (str == "MODERATE") return SignalStrength::MODERATE;
        if (str == "STRONG") return SignalStrength::STRONG;
        if (str == "VERY STRONG") return SignalStrength::VERY_STRONG;
        return SignalStrength::WEAK;
    }
    
    StrategyType stringToStrategyType(const std::string& str) {
        if (str == "ORB") return StrategyType::ORB;
        if (str == "VWAP") return StrategyType::VWAP;
        if (str == "MA Crossover") return StrategyType::MA_CROSSOVER;
        if (str == "RSI") return StrategyType::RSI;
        if (str == "Bollinger Bands") return StrategyType::BOLLINGER;
        if (str == "Breakout") return StrategyType::BREAKOUT;
        if (str == "Support/Resistance") return StrategyType::SUPPORT_RESISTANCE;
        if (str == "Momentum") return StrategyType::MOMENTUM;
        if (str == "Volume Spike") return StrategyType::VOLUME_SPIKE;
        if (str == "Reversal") return StrategyType::REVERSAL;
        return StrategyType::ORB;
    }
    
    double calculatePositionSize(double account_size, double risk_percentage, 
                               double entry_price, double stop_loss) {
        if (entry_price <= 0 || stop_loss <= 0 || entry_price == stop_loss) {
            return 0.0;
        }
        
        double risk_amount = account_size * (risk_percentage / 100.0);
        double risk_per_share = std::abs(entry_price - stop_loss);
        
        return std::floor(risk_amount / risk_per_share);
    }
    
    double calculateRiskRewardRatio(double entry, double stop_loss, double target) {
        if (entry <= 0 || stop_loss <= 0 || target <= 0 || entry == stop_loss) {
            return 0.0;
        }
        
        double risk = std::abs(entry - stop_loss);
        double reward = std::abs(target - entry);
        
        return reward / risk;
    }
    
    bool isValidSignal(const TradingSignal& signal) {
        // Basic validation checks
        if (signal.symbol.empty()) return false;
        if (signal.entry_price <= 0) return false;
        if (signal.stop_loss <= 0) return false;
        if (signal.confidence < 0 || signal.confidence > 1) return false;
        
        // Check if stop loss makes sense relative to entry
        if (signal.type == SignalType::BUY || signal.type == SignalType::STRONG_BUY) {
            if (signal.stop_loss >= signal.entry_price) return false;
        } else if (signal.type == SignalType::SELL || signal.type == SignalType::STRONG_SELL) {
            if (signal.stop_loss <= signal.entry_price) return false;
        }
        
        // Check if targets make sense
        if (signal.target_1 > 0) {
            if (signal.type == SignalType::BUY || signal.type == SignalType::STRONG_BUY) {
                if (signal.target_1 <= signal.entry_price) return false;
            } else if (signal.type == SignalType::SELL || signal.type == SignalType::STRONG_SELL) {
                if (signal.target_1 >= signal.entry_price) return false;
            }
        }
        
        return true;
    }
    
    double calculateSignalScore(const TradingSignal& signal) {
        double score = 0.0;
        
        // Base confidence score (0-40 points)
        score += signal.confidence * 40;
        
        // Technical score (0-20 points)
        score += signal.technical_score * 20;
        
        // Volume confirmation (0-15 points)
        if (signal.volume_above_average) {
            score += signal.volume_confirmation * 15;
        }
        
        // Breakout confirmation (0-10 points)
        if (signal.breakout_confirmed) {
            score += 10;
        }
        
        // Risk-reward ratio bonus (0-15 points)
        double rr_ratio = calculateRiskRewardRatio(signal.entry_price, 
                                                  signal.stop_loss, 
                                                  signal.target_1);
        if (rr_ratio > 0) {
            score += std::min(15.0, rr_ratio * 3);
        }
        
        // Strength bonus
        switch (signal.strength) {
            case SignalStrength::VERY_STRONG: score *= 1.2; break;
            case SignalStrength::STRONG: score *= 1.1; break;
            case SignalStrength::MODERATE: score *= 1.0; break;
            case SignalStrength::WEAK: score *= 0.9; break;
        }
        
        return std::min(100.0, score);
    }
    
    bool isSignalActive(const TradingSignal& signal) {
        auto now = std::chrono::system_clock::now();
        return now <= signal.expiry;
    }
    
    bool isWithinTradingHours(const std::chrono::system_clock::time_point& time) {
        auto time_t = std::chrono::system_clock::to_time_t(time);
        auto tm = *std::localtime(&time_t);
        
        // Indian market hours: 9:15 AM to 3:30 PM IST
        int current_minutes = tm.tm_hour * 60 + tm.tm_min;
        int market_open = 9 * 60 + 15;   // 9:15 AM
        int market_close = 15 * 60 + 30; // 3:30 PM
        
        // Check if it's a weekend
        if (tm.tm_wday == 0 || tm.tm_wday == 6) {
            return false;
        }
        
        return current_minutes >= market_open && current_minutes <= market_close;
    }
    
    int getMinutesUntilExpiry(const TradingSignal& signal) {
        auto now = std::chrono::system_clock::now();
        if (now > signal.expiry) return 0;
        
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(signal.expiry - now);
        return static_cast<int>(duration.count());
    }
}
