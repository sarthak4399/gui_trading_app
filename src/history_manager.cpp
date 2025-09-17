#include "history_manager.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <iostream>

HistoryManager::HistoryManager() {
}

HistoryManager::~HistoryManager() {
    cleanup();
}

bool HistoryManager::initialize(const std::string& data_directory) {
    data_directory_ = data_directory;
    
    try {
        // Create data directory if it doesn't exist
        std::filesystem::create_directories(data_directory_);
        
        // Load existing history
        loadHistoryFromFile();
        
        std::cout << " History manager initialized with " << signal_history_.size() 
                  << " signals" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "History manager initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void HistoryManager::cleanup() {
    saveHistoryToFile();
}

void HistoryManager::recordSignal(const TradingSignal& signal) {
    signal_history_.push_back(signal);
    invalidateCache();
    
    // Auto-save periodically
    if (signal_history_.size() % 10 == 0) {
        saveHistoryToFile();
    }
}

void HistoryManager::recordSetup(const TradingSetup& setup) {
    setup_history_.push_back(setup);
}

std::vector<TradingSignal> HistoryManager::getSignalHistory(const std::string& date) {
    if (date.empty()) {
        return signal_history_;
    }
    
    std::vector<TradingSignal> filtered;
    for (const auto& signal : signal_history_) {
        std::string signal_date = formatDateString(signal.timestamp);
        if (signal_date == date) {
            filtered.push_back(signal);
        }
    }
    return filtered;
}

std::vector<TradingSetup> HistoryManager::getSetupHistory(const std::string& date) {
    if (date.empty()) {
        return setup_history_;
    }
    
    std::vector<TradingSetup> filtered;
    for (const auto& setup : setup_history_) {
        std::string setup_date = formatDateString(setup.created_at);
        if (setup_date == date) {
            filtered.push_back(setup);
        }
    }
    return filtered;
}

DailyTradingSummary HistoryManager::getDailySummary(const std::string& date) {
    DailyTradingSummary summary;
    
    std::string target_date = date.empty() ? formatDateString(std::chrono::system_clock::now()) : date;
    summary.date = target_date;
    
    auto daily_signals = getSignalHistory(target_date);
    auto daily_setups = getSetupHistory(target_date);
    
    summary.total_setups_generated = static_cast<int>(daily_setups.size());
    
    // Count high confidence setups
    for (const auto& setup : daily_setups) {
        if (setup.overall_confidence > 0.7) {
            summary.high_confidence_setups++;
        }
    }
    
    // Count signals above threshold
    for (const auto& signal : daily_signals) {
        if (signal.confidence > 0.6) {
            summary.signals_above_threshold++;
        }
        
        // Count by strategy
        summary.strategy_signal_count[signal.strategy]++;
        
        // Count by symbol
        summary.symbol_activity[signal.symbol]++;
    }
    
    return summary;
}

bool HistoryManager::loadHistoryFromFile() {
    std::string filename = data_directory_ + "/signal_history.csv";
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        // File doesn't exist yet, which is OK for first run
        return true;
    }
    
    std::string line;
    std::getline(file, line); // Skip header
    
    while (std::getline(file, line)) {
        // Parse CSV line into TradingSignal
        // This is a simplified implementation
        TradingSignal signal;
        signal.symbol = "SAMPLE"; // Would parse from CSV
        signal_history_.push_back(signal);
    }
    
    return true;
}

bool HistoryManager::saveHistoryToFile() {
    try {
        std::string filename = data_directory_ + "/signal_history.csv";
        std::ofstream file(filename);
        
        if (!file.is_open()) {
            return false;
        }
        
        // Write header
        file << "timestamp,symbol,type,strategy,entry_price,stop_loss,target_1,confidence\n";
        
        // Write signals
        for (const auto& signal : signal_history_) {
            auto time_t = std::chrono::system_clock::to_time_t(signal.timestamp);
            file << time_t << ","
                 << signal.symbol << ","
                 << static_cast<int>(signal.type) << ","
                 << static_cast<int>(signal.strategy) << ","
                 << signal.entry_price << ","
                 << signal.stop_loss << ","
                 << signal.target_1 << ","
                 << signal.confidence << "\n";
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving history: " << e.what() << std::endl;
        return false;
    }
}

std::string HistoryManager::formatDateString(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

void HistoryManager::updatePerformanceCache() {
    // Implementation would calculate performance metrics
    // This is a stub for now
}

void HistoryManager::invalidateCache() {
    // Clear performance cache when new data is added
    performance_cache_.clear();
}

size_t HistoryManager::getRecordCount() const {
    return signal_history_.size();
}

void HistoryManager::updateSignalResult(const std::string& signal_id, bool profitable, double return_pct) {
    // Find signal by ID and update result
    // This would be implemented in a full version
    (void)signal_id;
    (void)profitable;
    (void)return_pct;
}

std::vector<TradingSignal> HistoryManager::getSignalsBySymbol(const std::string& symbol) {
    std::vector<TradingSignal> result;
    for (const auto& signal : signal_history_) {
        if (signal.symbol == symbol) {
            result.push_back(signal);
        }
    }
    return result;
}

std::vector<TradingSignal> HistoryManager::getSignalsByStrategy(StrategyType strategy) {
    std::vector<TradingSignal> result;
    for (const auto& signal : signal_history_) {
        if (signal.strategy == strategy) {
            result.push_back(signal);
        }
    }
    return result;
}

std::map<StrategyType, SignalPerformance> HistoryManager::getStrategyPerformance() {
    if (performance_cache_.empty()) {
        updatePerformanceCache();
    }
    return performance_cache_;
}

std::vector<std::string> HistoryManager::getProfitableTradingDays() {
    std::vector<std::string> profitable_days;
    // Implementation would analyze historical performance
    return profitable_days;
}

bool HistoryManager::exportToCSV(const std::string& filename, const std::string& date_range) {
    (void)filename;
    (void)date_range;
    return true; // Stub implementation
}

bool HistoryManager::exportToJSON(const std::string& filename, const std::string& date_range) {
    (void)filename;
    (void)date_range;
    return true; // Stub implementation
}

void HistoryManager::cleanupOldRecords(int days_to_keep) {
    auto cutoff_time = std::chrono::system_clock::now() - std::chrono::hours(24 * days_to_keep);
    
    signal_history_.erase(
        std::remove_if(signal_history_.begin(), signal_history_.end(),
                      [cutoff_time](const TradingSignal& signal) {
                          return signal.timestamp < cutoff_time;
                      }),
        signal_history_.end());
}
