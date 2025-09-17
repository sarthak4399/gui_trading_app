#include "yahoo_finance_api.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <signal.h>

bool running = true;

void signalHandler(int signum) {
    std::cout << "\n🛑 Stopping live data display...\n";
    running = false;
}

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void displayStockTable(const std::map<std::string, StockData>& stocks) {
    clearScreen();
    
    std::cout << "┌─────────────────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│                    🚀 HFT LIVE MARKET DATA - NIFTY 50                          │\n";
    std::cout << "├─────────────────────────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ Symbol     │  Price   │  Change  │  Change% │   Volume   │  RSI │  Signal      │\n";
    std::cout << "├─────────────────────────────────────────────────────────────────────────────────┤\n";
    
    for (const auto& [symbol, stock] : stocks) {
        std::string clean_symbol = symbol.substr(0, symbol.find('.'));
        std::string change_color = stock.change >= 0 ? "\033[32m" : "\033[31m"; // Green/Red
        std::string reset_color = "\033[0m";
        
        std::string signal = stock.is_breakout ? "🚀 BUY" : (stock.is_breakdown ? "📉 SELL" : "➖ HOLD");
        
        std::cout << "│ " << std::left << std::setw(10) << clean_symbol 
                  << " │ " << change_color << std::right << std::setw(8) << std::fixed << std::setprecision(1) << stock.current_price << reset_color
                  << " │ " << change_color << std::right << std::setw(8) << std::fixed << std::setprecision(2) << stock.change << reset_color
                  << " │ " << change_color << std::right << std::setw(8) << std::fixed << std::setprecision(2) << stock.change_percent << "%" << reset_color
                  << " │ " << std::right << std::setw(10) << std::fixed << std::setprecision(0) << stock.volume
                  << " │ " << std::right << std::setw(4) << std::fixed << std::setprecision(0) << stock.rsi_14
                  << " │ " << std::left << std::setw(12) << signal << " │\n";
    }
    
    std::cout << "└─────────────────────────────────────────────────────────────────────────────────┘\n";
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << "\n📊 Last Update: " << std::put_time(std::localtime(&time_t), "%H:%M:%S") 
              << " | Press Ctrl+C to stop\n";
}

int main() {
    signal(SIGINT, signalHandler);
    
    std::cout << "🚀 Starting HFT Live Data Display...\n";
    std::cout << "⏳ Initializing Yahoo Finance API...\n";
    
    YahooFinanceAPI api;
    
    std::cout << "📡 Fetching live market data...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    while (running) {
        try {
            auto stocks = api.getAllNifty50Data();
            
            if (!stocks.empty()) {
                displayStockTable(stocks);
            } else {
                std::cout << "⚠️  No data received. Retrying...\n";
            }
            
        } catch (const std::exception& e) {
            std::cout << "❌ Error: " << e.what() << "\n";
        }
        
        // Update every 5 seconds
        for (int i = 0; i < 50 && running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    std::cout << "\n✅ Live data display stopped.\n";
    return 0;
}
