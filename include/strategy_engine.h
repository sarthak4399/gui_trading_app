#pragma once

#include "market_data.h"
#include "trading_signal.h"
#include "technical_indicators.h"
#include <vector>
#include <memory>
#include <map>
#include <functional>

/**
 * Base Strategy Interface
 */
class BaseStrategy {
public:
    BaseStrategy(const std::string& name, StrategyType type) 
        : strategy_name_(name), strategy_type_(type) {}
    virtual ~BaseStrategy() = default;
    
    virtual std::vector<TradingSignal> analyze(const StockData& stock, 
                                             const HistoricalData& historical) = 0;
    virtual bool isApplicable(const StockData& stock) const = 0;
    virtual double getMinConfidenceThreshold() const = 0;
    
    const std::string& getName() const { return strategy_name_; }
    StrategyType getType() const { return strategy_type_; }
    
protected:
    std::string strategy_name_;
    StrategyType strategy_type_;
};

/**
 * Opening Range Breakout Strategy
 */
class ORBStrategy : public BaseStrategy {
public:
    ORBStrategy() : BaseStrategy("Opening Range Breakout", StrategyType::ORB) {}
    
    std::vector<TradingSignal> analyze(const StockData& stock, 
                                     const HistoricalData& historical) override;
    bool isApplicable(const StockData& stock) const override;
    double getMinConfidenceThreshold() const override { return 0.6; }
    
private:
    struct ORBLevels {
        double orb_high;
        double orb_low;
        double range_size;
        bool is_valid;
        
        ORBLevels() : orb_high(0), orb_low(0), range_size(0), is_valid(false) {}
    };
    
    ORBLevels calculateORBLevels(const HistoricalData& historical, int minutes = 15);
    double calculateBreakoutProbability(const StockData& stock, const ORBLevels& levels);
};

/**
 * VWAP Strategy
 */
class VWAPStrategy : public BaseStrategy {
public:
    VWAPStrategy() : BaseStrategy("VWAP", StrategyType::VWAP) {}
    
    std::vector<TradingSignal> analyze(const StockData& stock, 
                                     const HistoricalData& historical) override;
    bool isApplicable(const StockData& stock) const override;
    double getMinConfidenceThreshold() const override { return 0.55; }
    
private:
    enum class VWAPSignalType { ABOVE_VWAP, BELOW_VWAP, VWAP_CROSS };
    VWAPSignalType getVWAPSignal(const StockData& stock);
    double calculateVWAPStrength(const StockData& stock);
};

/**
 * RSI Strategy  
 */
class RSIStrategy : public BaseStrategy {
public:
    RSIStrategy() : BaseStrategy("RSI", StrategyType::RSI) {}
    
    std::vector<TradingSignal> analyze(const StockData& stock, 
                                     const HistoricalData& historical) override;
    bool isApplicable(const StockData& stock) const override;
    double getMinConfidenceThreshold() const override { return 0.65; }
    
private:
    bool isOverbought(double rsi, double threshold = 70.0);
    bool isOversold(double rsi, double threshold = 30.0);
    double calculateDivergenceStrength(const HistoricalData& historical);
};

/**
 * Breakout Strategy
 */
class BreakoutStrategy : public BaseStrategy {
public:
    BreakoutStrategy() : BaseStrategy("Breakout", StrategyType::BREAKOUT) {}
    
    std::vector<TradingSignal> analyze(const StockData& stock, 
                                     const HistoricalData& historical) override;
    bool isApplicable(const StockData& stock) const override;
    double getMinConfidenceThreshold() const override { return 0.7; }
    
private:
    bool detectBreakout(const StockData& stock, const HistoricalData& historical);
    bool detectBreakdown(const StockData& stock, const HistoricalData& historical);
    double calculateBreakoutStrength(const StockData& stock, const HistoricalData& historical);
};

/**
 * Volume Spike Strategy
 */
class VolumeStrategy : public BaseStrategy {
public:
    VolumeStrategy() : BaseStrategy("Volume Spike", StrategyType::VOLUME_SPIKE) {}
    
    std::vector<TradingSignal> analyze(const StockData& stock, 
                                     const HistoricalData& historical) override;
    bool isApplicable(const StockData& stock) const override;
    double getMinConfidenceThreshold() const override { return 0.6; }
    
private:
    bool isVolumeSpike(const StockData& stock, double threshold = 1.5);
    bool isPriceVolumeAlignment(const StockData& stock);
    double calculateVolumeStrength(const StockData& stock);
};

/**
 * Main Strategy Engine
 */
class StrategyEngine {
public:
    StrategyEngine();
    ~StrategyEngine();
    
    // Strategy management
    void addStrategy(std::unique_ptr<BaseStrategy> strategy);
    void removeStrategy(const std::string& name);
    void enableStrategy(const std::string& name);
    void disableStrategy(const std::string& name);
    
    // Analysis methods
    std::vector<TradingSignal> analyzeStock(const StockData& stock, 
                                           const HistoricalData& historical);
    std::vector<TradingSetup> generateSetups(const std::vector<StockData>& stocks,
                                           const std::map<std::string, HistoricalData>& historical_data);
    
    MarketScan generateMarketScan(const std::map<std::string, StockData>& all_stocks,
                                 const std::map<std::string, HistoricalData>& historical_data);
    
    // Signal filtering and ranking
    std::vector<TradingSignal> filterSignalsByConfidence(const std::vector<TradingSignal>& signals,
                                                        double min_confidence = 0.6);
    std::vector<TradingSignal> rankSignalsByScore(const std::vector<TradingSignal>& signals);
    
    // Setup generation
    TradingSetup combineSignalsToSetup(const std::string& symbol,
                                      const std::vector<TradingSignal>& signals);
    
    // Performance tracking
    void updatePerformanceStats(const TradingSignal& signal, bool was_profitable, 
                               double return_percentage);
    std::map<StrategyType, SignalPerformance> getPerformanceStats() const;
    
    // Configuration
    void setGlobalConfidenceThreshold(double threshold);
    void setRiskRewardThreshold(double min_rr_ratio);
    void setMaxSignalsPerStock(int max_signals);
    
    // Status
    std::vector<std::string> getActiveStrategies() const;
    std::vector<std::string> getAvailableStrategies() const;
    
private:
    std::vector<std::unique_ptr<BaseStrategy>> strategies_;
    std::map<std::string, bool> strategy_enabled_;
    std::map<StrategyType, SignalPerformance> performance_stats_;
    
    // Configuration parameters
    double global_confidence_threshold_;
    double min_risk_reward_ratio_;
    int max_signals_per_stock_;
    
    // Helper methods
    void initializeDefaultStrategies();
    double calculateSetupConfidence(const std::vector<TradingSignal>& signals);
    bool isHighQualitySetup(const TradingSetup& setup);
    void updateSignalMetrics(TradingSignal& signal, const StockData& stock);
};
