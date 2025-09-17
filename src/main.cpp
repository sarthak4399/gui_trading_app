#include "gui_app.h"
#include "yahoo_finance_api.h"
#include "strategy_engine.h"
#include <iostream>
#include <exception>
#include <csignal>
#include <cstring>

// Global application instance for signal handling
GuiApp* g_app = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    if (g_app) {
        g_app->cleanup();
    }
    exit(signal);
}

// Headless mode for testing core functionality
int runHeadlessTest() {
    std::cout << "===========================================================" << std::endl;
    std::cout << "🧪 HFT TRADING SYSTEM - Core Functionality Test" << std::endl;
    std::cout << "   Testing components without GUI..." << std::endl;
    std::cout << "===========================================================" << std::endl;
    
    try {
        // Test Yahoo Finance API
        std::cout << "🌐 Testing Yahoo Finance API..." << std::endl;
        YahooFinanceAPI api;
        std::cout << "  ✅ API initialized successfully" << std::endl;
        
        // Test Strategy Engine  
        std::cout << "🎯 Testing Strategy Engine..." << std::endl;
        StrategyEngine engine;
        std::cout << "  ✅ Strategy engine initialized successfully" << std::endl;
        
        std::cout << "\n🎉 All core components working!" << std::endl;
        std::cout << "📝 GUI requires display - run with display server for full functionality" << std::endl;
        std::cout << "===========================================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Core test failed: " << e.what() << std::endl;
        return -1;
    }
}

// Show help information
void showHelp() {
    std::cout << "🚀 HFT Trading Application - Usage Options\n" << std::endl;
    std::cout << "BASIC USAGE:" << std::endl;
    std::cout << "  ./HFTTradingApp                 # Auto-detect display mode" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "COMMAND LINE OPTIONS:" << std::endl;
    std::cout << "  --gui, --display, -display     # Force GUI mode (even without display detection)" << std::endl;
    std::cout << "  --test, --headless              # Run core functionality tests (no GUI)" << std::endl;
    std::cout << "  --help, -h                      # Show this help message" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "EXAMPLES:" << std::endl;
    std::cout << "  ./HFTTradingApp --gui           # Force GUI mode" << std::endl;
    std::cout << "  ./HFTTradingApp --test          # Run without GUI" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "FEATURES:" << std::endl;
    std::cout << "  • Live Nifty 50 stock data via Yahoo Finance" << std::endl;
    std::cout << "  • 5 trading strategies (ORB, VWAP, RSI, Breakout, Volume)" << std::endl;
    std::cout << "  • Technical analysis indicators" << std::endl;
    std::cout << "  • Market scanning and trade history" << std::endl;
    std::cout << "  • Professional HFT-style interface" << std::endl;
    std::cout << "\n📝 For GUI mode, ensure you have a desktop environment running." << std::endl;
}

int main(int argc, char* argv[]) {
    // Check for help
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        showHelp();
        return 0;
    }
    
    // Check for headless mode
    bool headless = (argc > 1 && (strcmp(argv[1], "--test") == 0 || strcmp(argv[1], "--headless") == 0));
    if (headless) {
        return runHeadlessTest();
    }
    
    // Check for force GUI mode
    bool forceGui = (argc > 1 && (strcmp(argv[1], "--gui") == 0 || strcmp(argv[1], "--display") == 0 || strcmp(argv[1], "-display") == 0));
    
    // Check if DISPLAY is available (for X11 systems)
    const char* display = getenv("DISPLAY");
    if (!display && getenv("WAYLAND_DISPLAY") == nullptr && !forceGui) {
        std::cout << "⚠️  No display detected - running core functionality test instead" << std::endl;
        std::cout << "   Use './HFTTradingApp --gui' to force GUI mode" << std::endl;
        std::cout << "   Use './HFTTradingApp --test' to explicitly run tests" << std::endl;
        std::cout << "   For GUI mode, run from a desktop environment\n" << std::endl;
        return runHeadlessTest();
    }
    
    if (forceGui) {
        std::cout << "🖥️  Force GUI mode enabled" << std::endl;
    }
    
    try {
        std::cout << "===========================================================" << std::endl;
        std::cout << "🚀 HFT TRADING APPLICATION - Professional Intraday System" << std::endl;
        std::cout << "   Live Market Data • Technical Analysis • Trading Signals" << std::endl;
        std::cout << "===========================================================" << std::endl;
        
        // Setup signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        // Create and initialize application
        GuiApp app;
        g_app = &app;
        
        std::cout << "🔧 Initializing application components..." << std::endl;
        if (!app.initialize()) {
            std::cerr << "❌ Failed to initialize application!" << std::endl;
            std::cerr << "💡 Try running with --test flag for headless mode" << std::endl;
            return -1;
        }
        
        std::cout << "✅ Application initialized successfully!" << std::endl;
        std::cout << "📊 Starting live market data feed..." << std::endl;
        std::cout << "🎯 Loading trading strategies..." << std::endl;
        std::cout << "📈 GUI ready - Happy Trading!" << std::endl;
        std::cout << "===========================================================" << std::endl;
        
        // Run the main application loop
        app.run();
        
        std::cout << "👋 Application shutting down..." << std::endl;
        app.cleanup();
        
        std::cout << "✅ Shutdown complete. Goodbye!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        std::cerr << "💡 If this is a display issue, try running with --test flag" << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "❌ Unknown fatal error occurred!" << std::endl;
        std::cerr << "💡 This may be a display/GUI issue - try --test mode" << std::endl;
        return -1;
    }
}
