#pragma once

#include "trading_signal.h"
#include <string>
#include <vector>
#include <chrono>
#include <map>

/**
 * History Manager for storing and retrieving trading history
 */
class HistoryManager {
public:
    HistoryManager();
    ~HistoryManager();
    
    // Initialization
    bool initialize(const std::string& data_directory = "data");
    void cleanup();
    
    // Signal history
    void recordSignal(const TradingSignal& signal);
    void recordSetup(const TradingSetup& setup);
    void updateSignalResult(const std::string& signal_id, bool profitable, double return_pct);
    
    // Query methods
    std::vector<TradingSignal> getSignalHistory(const std::string& date = "");
    std::vector<TradingSetup> getSetupHistory(const std::string& date = "");
    std::vector<TradingSignal> getSignalsBySymbol(const std::string& symbol);
    std::vector<TradingSignal> getSignalsByStrategy(StrategyType strategy);
    
    // Performance analytics
    DailyTradingSummary getDailySummary(const std::string& date = "");
    std::map<StrategyType, SignalPerformance> getStrategyPerformance();
    std::vector<std::string> getProfitableTradingDays();
    
    // Export functionality
    bool exportToCSV(const std::string& filename, const std::string& date_range = "");
    bool exportToJSON(const std::string& filename, const std::string& date_range = "");
    
    // Maintenance
    void cleanupOldRecords(int days_to_keep = 90);
    size_t getRecordCount() const;
    
private:
    std::string data_directory_;
    std::vector<TradingSignal> signal_history_;
    std::vector<TradingSetup> setup_history_;
    std::map<StrategyType, SignalPerformance> performance_cache_;
    
    // File operations
    bool loadHistoryFromFile();
    bool saveHistoryToFile();
    std::string generateSignalId(const TradingSignal& signal);
    std::string formatDateString(const std::chrono::system_clock::time_point& time);
    
    // Cache management
    void updatePerformanceCache();
    void invalidateCache();
};
