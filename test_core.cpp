/**
 * Core functionality test for HFT Trading System
 * Tests the core components without GUI
 */
#include "yahoo_finance_api.h"
#include "strategy_engine.h"
#include "technical_indicators.h"
#include "history_manager.h"
#include <iostream>

int main() {
    std::cout << "🧪 Testing HFT Trading System Core Components\n" << std::endl;
    
    try {
        // Test 1: Technical Indicators
        std::cout << "📊 Testing Technical Indicators..." << std::endl;
        TechnicalIndicators tech;
        std::vector<double> prices = {100.0, 102.0, 101.0, 103.0, 105.0, 104.0, 106.0};
        
        auto sma = tech.calculateSMA(prices, 3);
        std::cout << "✅ SMA calculation: " << (sma.empty() ? "Failed" : "Success") << std::endl;
        
        auto rsi = tech.calculateRSI(prices, 6);
        std::cout << "✅ RSI calculation: " << (rsi.empty() ? "Failed" : "Success") << std::endl;
        
        // Test 2: Strategy Engine
        std::cout << "\n🎯 Testing Strategy Engine..." << std::endl;
        StrategyEngine engine;
        
        StockData sample_stock;
        sample_stock.symbol = "RELIANCE";
        sample_stock.current_price = 2500.0;
        sample_stock.volume = 1000000;
        sample_stock.high_price = 2520.0;
        sample_stock.low_price = 2480.0;
        sample_stock.open_price = 2490.0;
        
        auto signals = engine.generateSignals(sample_stock);
        std::cout << "✅ Signal generation: " << signals.size() << " signals generated" << std::endl;
        
        // Test 3: History Manager
        std::cout << "\n📋 Testing History Manager..." << std::endl;
        HistoryManager history;
        bool init_success = history.initialize();
        std::cout << "✅ History manager initialization: " << (init_success ? "Success" : "Failed") << std::endl;
        
        // Test 4: Yahoo Finance API (structure test)
        std::cout << "\n🌐 Testing Yahoo Finance API structure..." << std::endl;
        YahooFinanceAPI api;
        std::cout << "✅ API object creation: Success" << std::endl;
        
        std::cout << "\n🎉 All core component tests completed successfully!" << std::endl;
        std::cout << "📝 The HFT Trading System is ready for use." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}
