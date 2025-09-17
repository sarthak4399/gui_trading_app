#include "strategy_engine.h"
#include <algorithm>
#include <numeric>
#include <iostream>

// StrategyEngine Implementation
StrategyEngine::StrategyEngine() 
    : global_confidence_threshold_(0.6), 
      min_risk_reward_ratio_(1.5), 
      max_signals_per_stock_(3) {
    initializeDefaultStrategies();
}

StrategyEngine::~StrategyEngine() = default;

void StrategyEngine::initializeDefaultStrategies() {
    addStrategy(std::make_unique<ORBStrategy>());
    addStrategy(std::make_unique<VWAPStrategy>());
    addStrategy(std::make_unique<RSIStrategy>());
    addStrategy(std::make_unique<BreakoutStrategy>());
    addStrategy(std::make_unique<VolumeStrategy>());
    
    std::cout << "✅ Strategy engine initialized with " << strategies_.size() 
              << " strategies" << std::endl;
}

void StrategyEngine::addStrategy(std::unique_ptr<BaseStrategy> strategy) {
    std::string name = strategy->getName();
    strategies_.push_back(std::move(strategy));
    strategy_enabled_[name] = true;
}

std::vector<TradingSignal> StrategyEngine::analyzeStock(const StockData& stock, 
                                                       const HistoricalData& historical) {
    std::vector<TradingSignal> all_signals;
    
    for (const auto& strategy : strategies_) {
        if (!strategy_enabled_[strategy->getName()]) continue;
        
        if (strategy->isApplicable(stock)) {
            auto signals = strategy->analyze(stock, historical);
            for (auto& signal : signals) {
                updateSignalMetrics(signal, stock);
                if (signal.confidence >= strategy->getMinConfidenceThreshold()) {
                    all_signals.push_back(signal);
                }
            }
        }
    }
    
    // Limit signals per stock
    if (all_signals.size() > static_cast<size_t>(max_signals_per_stock_)) {
        std::sort(all_signals.begin(), all_signals.end(),
                 [](const TradingSignal& a, const TradingSignal& b) {
                     return a.confidence > b.confidence;
                 });
        all_signals.resize(max_signals_per_stock_);
    }
    
    return all_signals;
}

std::vector<TradingSetup> StrategyEngine::generateSetups(const std::vector<StockData>& stocks,
                                                        const std::map<std::string, HistoricalData>& historical_data) {
    std::vector<TradingSetup> setups;
    
    for (const auto& stock : stocks) {
        auto hist_it = historical_data.find(stock.symbol);
        if (hist_it == historical_data.end()) continue;
        
        auto signals = analyzeStock(stock, hist_it->second);
        if (!signals.empty()) {
            auto setup = combineSignalsToSetup(stock.symbol, signals);
            if (isHighQualitySetup(setup)) {
                setups.push_back(setup);
            }
        }
    }
    
    // Sort by confidence
    std::sort(setups.begin(), setups.end(),
             [](const TradingSetup& a, const TradingSetup& b) {
                 return a.overall_confidence > b.overall_confidence;
             });
    
    return setups;
}

TradingSetup StrategyEngine::combineSignalsToSetup(const std::string& symbol,
                                                  const std::vector<TradingSignal>& signals) {
    TradingSetup setup;
    setup.symbol = symbol;
    setup.supporting_signals = signals;
    setup.technical_confluence = static_cast<int>(signals.size());
    
    if (signals.empty()) return setup;
    
    // Use highest confidence signal as primary
    auto primary_it = std::max_element(signals.begin(), signals.end(),
                                     [](const TradingSignal& a, const TradingSignal& b) {
                                         return a.confidence < b.confidence;
                                     });
    
    setup.primary_signal = primary_it->type;
    setup.recommended_entry = primary_it->entry_price;
    setup.stop_loss = primary_it->stop_loss;
    setup.target_1 = primary_it->target_1;
    setup.target_2 = primary_it->target_2;
    
    // Calculate combined confidence
    setup.overall_confidence = calculateSetupConfidence(signals);
    
    // Calculate risk-reward
    setup.risk_reward_ratio = SignalUtils::calculateRiskRewardRatio(
        setup.recommended_entry, setup.stop_loss, setup.target_1);
    
    // Set validity period
    setup.setup_validity_minutes = 30;
    setup.best_entry_time = "Market Open +15 mins";
    
    return setup;
}

double StrategyEngine::calculateSetupConfidence(const std::vector<TradingSignal>& signals) {
    if (signals.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& signal : signals) {
        sum += signal.confidence;
    }
    
    // Average confidence with bonus for multiple signals
    double avg_confidence = sum / signals.size();
    double confluence_bonus = std::min(0.2, (signals.size() - 1) * 0.05);
    
    return std::min(1.0, avg_confidence + confluence_bonus);
}

bool StrategyEngine::isHighQualitySetup(const TradingSetup& setup) {
    return setup.overall_confidence >= global_confidence_threshold_ &&
           setup.risk_reward_ratio >= min_risk_reward_ratio_;
}

void StrategyEngine::updateSignalMetrics(TradingSignal& signal, const StockData& stock) {
    signal.symbol = stock.symbol;
    signal.volume_above_average = stock.volume_spike;
    signal.breakout_confirmed = stock.is_breakout || stock.is_breakdown;
    signal.technical_score = std::min(1.0, (stock.rsi_14 / 50.0) * 0.5 + 0.5);
}

std::vector<std::string> StrategyEngine::getActiveStrategies() const {
    std::vector<std::string> active;
    for (const auto& [name, enabled] : strategy_enabled_) {
        if (enabled) active.push_back(name);
    }
    return active;
}

// ORB Strategy Implementation
std::vector<TradingSignal> ORBStrategy::analyze(const StockData& stock, const HistoricalData& historical) {
    std::vector<TradingSignal> signals;
    
    auto orb_levels = calculateORBLevels(historical);
    if (!orb_levels.is_valid) return signals;
    
    TradingSignal signal;
    signal.strategy = StrategyType::ORB;
    signal.symbol = stock.symbol;
    
    // Check for breakout above ORB high
    if (stock.current_price > orb_levels.orb_high) {
        signal.type = SignalType::BUY;
        signal.entry_price = orb_levels.orb_high;
        signal.stop_loss = orb_levels.orb_low;
        signal.target_1 = orb_levels.orb_high + (orb_levels.range_size * 1.5);
        signal.confidence = calculateBreakoutProbability(stock, orb_levels);
        signal.setup_description = "ORB Breakout - Price above opening range high";
        signals.push_back(signal);
    }
    // Check for breakdown below ORB low
    else if (stock.current_price < orb_levels.orb_low) {
        signal.type = SignalType::SELL;
        signal.entry_price = orb_levels.orb_low;
        signal.stop_loss = orb_levels.orb_high;
        signal.target_1 = orb_levels.orb_low - (orb_levels.range_size * 1.5);
        signal.confidence = calculateBreakoutProbability(stock, orb_levels);
        signal.setup_description = "ORB Breakdown - Price below opening range low";
        signals.push_back(signal);
    }
    
    return signals;
}

bool ORBStrategy::isApplicable(const StockData& stock) const {
    return stock.volume_ratio > 1.2 && stock.current_price > 100; // Minimum criteria
}

ORBStrategy::ORBLevels ORBStrategy::calculateORBLevels(const HistoricalData& historical, int minutes) {
    ORBLevels levels;
    (void)minutes; // Suppress unused parameter warning
    
    if (historical.candles.size() < 5) return levels;
    
    // Use first few candles as opening range
    double high = 0, low = std::numeric_limits<double>::max();
    for (size_t i = 0; i < std::min(size_t(5), historical.candles.size()); ++i) {
        high = std::max(high, historical.candles[i].high);
        low = std::min(low, historical.candles[i].low);
    }
    
    levels.orb_high = high;
    levels.orb_low = low;
    levels.range_size = high - low;
    levels.is_valid = levels.range_size > 0;
    
    return levels;
}

double ORBStrategy::calculateBreakoutProbability(const StockData& stock, const ORBLevels& levels) {
    double base_prob = 0.5;
    
    // Volume confirmation
    if (stock.volume_ratio > 1.5) base_prob += 0.2;
    
    // Range size factor
    double range_pct = levels.range_size / stock.current_price;
    if (range_pct > 0.02) base_prob += 0.1;
    
    return std::min(0.9, base_prob);
}

// VWAP Strategy Implementation
std::vector<TradingSignal> VWAPStrategy::analyze(const StockData& stock, const HistoricalData& historical) {
    (void)historical; // Suppress unused parameter warning
    std::vector<TradingSignal> signals;
    
    if (stock.vwap <= 0) return signals;
    
    TradingSignal signal;
    signal.strategy = StrategyType::VWAP;
    signal.symbol = stock.symbol;
    
    double price_diff_pct = (stock.current_price - stock.vwap) / stock.vwap;
    
    if (price_diff_pct > 0.01 && stock.volume_ratio > 1.3) {
        // Price above VWAP with volume
        signal.type = SignalType::BUY;
        signal.entry_price = stock.current_price;
        signal.stop_loss = stock.vwap;
        signal.target_1 = stock.current_price * 1.02;
        signal.confidence = calculateVWAPStrength(stock);
        signal.setup_description = "VWAP Breakout - Price above VWAP with volume";
        signals.push_back(signal);
    } else if (price_diff_pct < -0.01 && stock.volume_ratio > 1.3) {
        // Price below VWAP with volume
        signal.type = SignalType::SELL;
        signal.entry_price = stock.current_price;
        signal.stop_loss = stock.vwap;
        signal.target_1 = stock.current_price * 0.98;
        signal.confidence = calculateVWAPStrength(stock);
        signal.setup_description = "VWAP Breakdown - Price below VWAP with volume";
        signals.push_back(signal);
    }
    
    return signals;
}

bool VWAPStrategy::isApplicable(const StockData& stock) const {
    return stock.vwap > 0 && stock.volume > 0;
}

double VWAPStrategy::calculateVWAPStrength(const StockData& stock) {
    double base_strength = 0.5;
    
    // Volume factor
    if (stock.volume_ratio > 1.5) base_strength += 0.2;
    
    // Price distance from VWAP
    double distance = std::abs(stock.current_price - stock.vwap) / stock.vwap;
    if (distance > 0.01) base_strength += 0.1;
    
    return std::min(0.8, base_strength);
}

// RSI Strategy Implementation (stub)
std::vector<TradingSignal> RSIStrategy::analyze(const StockData& stock, const HistoricalData& historical) {
    (void)historical;
    std::vector<TradingSignal> signals;
    
    TradingSignal signal;
    signal.strategy = StrategyType::RSI;
    signal.symbol = stock.symbol;
    
    if (isOversold(stock.rsi_14)) {
        signal.type = SignalType::BUY;
        signal.confidence = 0.6;
        signal.setup_description = "RSI Oversold";
        signals.push_back(signal);
    } else if (isOverbought(stock.rsi_14)) {
        signal.type = SignalType::SELL;
        signal.confidence = 0.6;
        signal.setup_description = "RSI Overbought";
        signals.push_back(signal);
    }
    
    return signals;
}

bool RSIStrategy::isApplicable(const StockData& stock) const {
    return stock.rsi_14 > 0 && stock.rsi_14 < 100;
}

bool RSIStrategy::isOverbought(double rsi, double threshold) {
    return rsi > threshold;
}

bool RSIStrategy::isOversold(double rsi, double threshold) {
    return rsi < threshold;
}

double RSIStrategy::calculateDivergenceStrength(const HistoricalData& historical) {
    (void)historical;
    return 0.5; // Stub
}

// Breakout Strategy Implementation (stub)
std::vector<TradingSignal> BreakoutStrategy::analyze(const StockData& stock, const HistoricalData& historical) {
    std::vector<TradingSignal> signals;
    
    if (detectBreakout(stock, historical)) {
        TradingSignal signal;
        signal.strategy = StrategyType::BREAKOUT;
        signal.symbol = stock.symbol;
        signal.type = SignalType::BUY;
        signal.confidence = 0.7;
        signal.setup_description = "Price Breakout";
        signals.push_back(signal);
    }
    
    return signals;
}

bool BreakoutStrategy::isApplicable(const StockData& stock) const {
    return stock.volume_ratio > 1.0;
}

bool BreakoutStrategy::detectBreakout(const StockData& stock, const HistoricalData& historical) {
    (void)historical;
    return stock.is_breakout;
}

bool BreakoutStrategy::detectBreakdown(const StockData& stock, const HistoricalData& historical) {
    (void)historical;
    return stock.is_breakdown;
}

double BreakoutStrategy::calculateBreakoutStrength(const StockData& stock, const HistoricalData& historical) {
    (void)stock;
    (void)historical;
    return 0.7; // Stub
}

// Volume Strategy Implementation (stub)
std::vector<TradingSignal> VolumeStrategy::analyze(const StockData& stock, const HistoricalData& historical) {
    (void)historical;
    std::vector<TradingSignal> signals;
    
    if (isVolumeSpike(stock) && isPriceVolumeAlignment(stock)) {
        TradingSignal signal;
        signal.strategy = StrategyType::VOLUME_SPIKE;
        signal.symbol = stock.symbol;
        signal.type = stock.change > 0 ? SignalType::BUY : SignalType::SELL;
        signal.confidence = calculateVolumeStrength(stock);
        signal.setup_description = "Volume Spike with Price Alignment";
        signals.push_back(signal);
    }
    
    return signals;
}

bool VolumeStrategy::isApplicable(const StockData& stock) const {
    return stock.volume > 0 && stock.avg_volume > 0;
}

bool VolumeStrategy::isVolumeSpike(const StockData& stock, double threshold) {
    return stock.volume_ratio > threshold;
}

bool VolumeStrategy::isPriceVolumeAlignment(const StockData& stock) {
    return (stock.change > 0 && stock.volume_spike) || 
           (stock.change < 0 && stock.volume_spike);
}

double VolumeStrategy::calculateVolumeStrength(const StockData& stock) {
    return std::min(0.8, stock.volume_ratio * 0.3);
}

// Additional StrategyEngine methods (stubs)
void StrategyEngine::removeStrategy(const std::string& name) {
    strategies_.erase(
        std::remove_if(strategies_.begin(), strategies_.end(),
                      [&name](const auto& strategy) {
                          return strategy->getName() == name;
                      }),
        strategies_.end());
}

void StrategyEngine::enableStrategy(const std::string& name) {
    strategy_enabled_[name] = true;
}

void StrategyEngine::disableStrategy(const std::string& name) {
    strategy_enabled_[name] = false;
}

MarketScan StrategyEngine::generateMarketScan(const std::map<std::string, StockData>& all_stocks,
                                            const std::map<std::string, HistoricalData>& historical_data) {
    (void)all_stocks;
    (void)historical_data;
    return MarketScan{}; // Stub
}

std::vector<TradingSignal> StrategyEngine::filterSignalsByConfidence(const std::vector<TradingSignal>& signals,
                                                                   double min_confidence) {
    std::vector<TradingSignal> filtered;
    std::copy_if(signals.begin(), signals.end(), std::back_inserter(filtered),
                [min_confidence](const TradingSignal& signal) {
                    return signal.confidence >= min_confidence;
                });
    return filtered;
}

std::vector<TradingSignal> StrategyEngine::rankSignalsByScore(const std::vector<TradingSignal>& signals) {
    auto ranked = signals;
    std::sort(ranked.begin(), ranked.end(),
             [](const TradingSignal& a, const TradingSignal& b) {
                 return SignalUtils::calculateSignalScore(a) > SignalUtils::calculateSignalScore(b);
             });
    return ranked;
}

void StrategyEngine::updatePerformanceStats(const TradingSignal& signal, bool was_profitable, double return_percentage) {
    (void)signal;
    (void)was_profitable;
    (void)return_percentage;
    // Implementation would update performance_stats_
}

std::map<StrategyType, SignalPerformance> StrategyEngine::getPerformanceStats() const {
    return performance_stats_;
}

void StrategyEngine::setGlobalConfidenceThreshold(double threshold) {
    global_confidence_threshold_ = threshold;
}

void StrategyEngine::setRiskRewardThreshold(double min_rr_ratio) {
    min_risk_reward_ratio_ = min_rr_ratio;
}

void StrategyEngine::setMaxSignalsPerStock(int max_signals) {
    max_signals_per_stock_ = max_signals;
}

std::vector<std::string> StrategyEngine::getAvailableStrategies() const {
    std::vector<std::string> available;
    for (const auto& strategy : strategies_) {
        available.push_back(strategy->getName());
    }
    return available;
}
