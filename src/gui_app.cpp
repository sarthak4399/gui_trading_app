#include "gui_app.h"
#include "data_fetcher.h"
#include <iostream>
#include <iomanip>

GuiApp::GuiApp() : running_(false), data_update_active_(false) {
}

GuiApp::~GuiApp() {
    cleanup();
}

bool GuiApp::initialize() {
    try {
        std::cout << "🎨 Setting up GUI window..." << std::endl;
        setupWindow();
        
        std::cout << "🔧 Initializing components..." << std::endl;
        
        // Initialize API and data components
        std::cout << "  • Creating Yahoo Finance API..." << std::endl;
        api_ = std::make_unique<YahooFinanceAPI>();
        
        std::cout << "  • Creating Strategy Engine..." << std::endl;
        strategy_engine_ = std::make_unique<StrategyEngine>();
        
        std::cout << "  • Creating History Manager..." << std::endl;
        history_manager_ = std::make_unique<HistoryManager>();
        
        std::cout << "  • Initializing History Manager..." << std::endl;
        if (!history_manager_->initialize()) {
            std::cerr << "❌ Failed to initialize history manager" << std::endl;
            return false;
        }
        
        std::cout << "📊 Setting up GUI components..." << std::endl;
        setupGUI();
        
        std::cout << "🚀 Starting data feed..." << std::endl;
        startDataUpdateThread();
        
        return true;
    } catch (const std::runtime_error& e) {
        std::cerr << "❌ GUI initialization failed (runtime error): " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "❌ GUI initialization failed (exception): " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "❌ GUI initialization failed: Completely unknown error during initialization" << std::endl;
        return false;
    }
}

void GuiApp::setupWindow() {
    std::cout << "  • Creating SFML window..." << std::endl;
    
    // Try smaller window size first to ensure it works
    const unsigned int width = 800;  // Smaller window for testing
    const unsigned int height = 600;
    
    std::cout << "  • Using window size: " << width << "x" << height << std::endl;
    
    try {
        // Create window with simple parameters
        window_.create(sf::VideoMode({width, height}), 
                       "HFT Trading System",  // Simpler title
                       sf::Style::Default);
        
        std::cout << "  • Window.create() completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception during window_.create(): " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cout << "❌ Unknown exception during window_.create()" << std::endl;
        throw;
    }
    
    if (!window_.isOpen()) {
        throw std::runtime_error("Window failed to open after creation");
    }
    
    std::cout << "  ✅ Window is now open!" << std::endl;
    
    // Make window visible and bring to front
    window_.setVisible(true);
    std::cout << "  • Window set visible" << std::endl;
    
    // Set basic properties
    window_.setFramerateLimit(60);
    std::cout << "  • Frame rate limit set" << std::endl;
    
    // Initialize TGUI
    std::cout << "  • Initializing TGUI..." << std::endl;
    gui_ = std::make_unique<tgui::Gui>(window_);
    std::cout << "  • TGUI initialized" << std::endl;
    
    // Don't center for now - just use default position
    // centerWindow();
    
    std::cout << "  ✅ GUI window setup complete!" << std::endl;
    std::cout << "  📱 Window should now be visible on your screen!" << std::endl;
}

void GuiApp::setupGUI() {
    setupMenuBar();
    setupMainTabs();
    setupStatusBar();
    
    // Load theme
    loadTheme(config_.theme_name);
}

void GuiApp::setupMenuBar() {
    menu_bar_ = tgui::MenuBar::create();
    menu_bar_->setPosition(0, 0);
    menu_bar_->setSize("100%", 25);
    
        // File menu
    menu_bar_->addMenu("File");
    menu_bar_->addMenuItem({"File", "Exit"});
    menu_bar_->addMenuItem({"File", "Refresh Data"});
    menu_bar_->addMenuItem({"File", "Settings"});
    menu_bar_->addMenuItem({"File", "Export History"});
    
    // Tools menu
    menu_bar_->addMenu("Tools");
    menu_bar_->addMenuItem({"Tools", "Risk Calculator"});
    menu_bar_->addMenuItem({"Tools", "Performance Analytics"});
    
    // Help menu
    menu_bar_->addMenu("Help");
    menu_bar_->addMenuItem({"Help", "About"});
    menu_bar_->addMenuItem({"Help", "User Guide"});
    
    menu_bar_->connectMenuItem({"File", "Exit"}, [this]() { window_.close(); });
    menu_bar_->connectMenuItem({"File", "Refresh Data"}, [this]() { onRefreshClicked(); });
    menu_bar_->connectMenuItem({"File", "Settings"}, [this]() { onSettingsClicked(); });
    menu_bar_->connectMenuItem({"File", "Export History"}, [this]() { onExportHistory(); });
    
    gui_->add(menu_bar_);
}

void GuiApp::setupMainTabs() {
    tab_container_ = tgui::TabContainer::create();
    tab_container_->setPosition(0, 25);
    tab_container_->setSize("100%", "100% - 75");
    
    setupLiveDataTab();
    setupMarketScanTab();
    setupHistoryTab();
    
    gui_->add(tab_container_);
}

void GuiApp::setupLiveDataTab() {
    live_panel_ = tgui::Panel::create();
    live_panel_->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
    
    // Create main horizontal split
    auto main_split = tgui::Panel::create();
    main_split->setSize("100%", "100%");
    main_split->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
    
    // Stocks table (left side)
    stocks_table_ = tgui::ListView::create();
    stocks_table_->setPosition("1%", "1%");
    stocks_table_->setSize("65%", "98%");
    
    // Add columns
    stocks_table_->addColumn("Symbol", 80);
    stocks_table_->addColumn("Price", 80);
    stocks_table_->addColumn("Change", 80);
    stocks_table_->addColumn("Change%", 80);
    stocks_table_->addColumn("Volume", 100);
    stocks_table_->addColumn("RSI", 60);
    stocks_table_->addColumn("Signal", 80);
    
    stocks_table_->onItemSelect([this](int index) { 
        (void)index; 
        onStockSelected(); 
    });
    
    main_split->add(stocks_table_);
    
    // Setups panel (right side)
    setups_panel_ = tgui::Panel::create();
    setups_panel_->setPosition("67%", "1%");
    setups_panel_->setSize("32%", "98%");
    setups_panel_->getRenderer()->setBackgroundColor(tgui::Color(30, 30, 30));
    setups_panel_->getRenderer()->setBorderColor(tgui::Color::White);
    setups_panel_->getRenderer()->setBorders(1);
    
    auto setups_title = tgui::Label::create("🎯 TRADING SETUPS");
    setups_title->setPosition("2%", "2%");
    setups_title->setTextSize(16);
    setups_title->getRenderer()->setTextColor(tgui::Color::Yellow);
    setups_panel_->add(setups_title);
    
    setups_list_ = tgui::ListView::create();
    setups_list_->setPosition("2%", "8%");
    setups_list_->setSize("96%", "90%");
    setups_list_->addColumn("Symbol", 60);
    setups_list_->addColumn("Type", 60);
    setups_list_->addColumn("Entry", 70);
    setups_list_->addColumn("R:R", 50);
    setups_list_->addColumn("Conf%", 50);
    
    setups_panel_->add(setups_list_);
    main_split->add(setups_panel_);
    
    live_panel_->add(main_split);
    auto live_tab = tab_container_->addTab("📊 Live Data", false);
    live_tab->add(live_panel_);
}

void GuiApp::setupMarketScanTab() {
    scan_panel_ = tgui::Panel::create();
    scan_panel_->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
    
    // Create three columns for gainers, losers, and high volume
    auto col_width = "32%";
    auto col_spacing = "34%";
    
    // Top Gainers
    auto gainers_title = tgui::Label::create("📈 TOP GAINERS");
    gainers_title->setPosition("2%", "2%");
    gainers_title->setTextSize(14);
    gainers_title->getRenderer()->setTextColor(tgui::Color::Green);
    scan_panel_->add(gainers_title);
    
    gainers_list_ = tgui::ListView::create();
    gainers_list_->setPosition("2%", "6%");
    gainers_list_->setSize(col_width, "90%");
    gainers_list_->addColumn("Symbol", 80);
    gainers_list_->addColumn("Change%", 80);
    gainers_list_->addColumn("Price", 80);
    scan_panel_->add(gainers_list_);
    
    // Top Losers
    auto losers_title = tgui::Label::create("📉 TOP LOSERS");
    losers_title->setPosition(col_spacing, "2%");
    losers_title->setTextSize(14);
    losers_title->getRenderer()->setTextColor(tgui::Color::Red);
    scan_panel_->add(losers_title);
    
    losers_list_ = tgui::ListView::create();
    losers_list_->setPosition(col_spacing, "6%");
    losers_list_->setSize(col_width, "90%");
    losers_list_->addColumn("Symbol", 80);
    losers_list_->addColumn("Change%", 80);
    losers_list_->addColumn("Price", 80);
    scan_panel_->add(losers_list_);
    
    // High Volume
    auto volume_title = tgui::Label::create("📊 HIGH VOLUME");
    volume_title->setPosition("68%", "2%");
    volume_title->setTextSize(14);
    volume_title->getRenderer()->setTextColor(tgui::Color::Cyan);
    scan_panel_->add(volume_title);
    
    high_volume_list_ = tgui::ListView::create();
    high_volume_list_->setPosition("68%", "6%");
    high_volume_list_->setSize(col_width, "90%");
    high_volume_list_->addColumn("Symbol", 80);
    high_volume_list_->addColumn("Volume", 100);
    high_volume_list_->addColumn("Ratio", 60);
    scan_panel_->add(high_volume_list_);
    
    auto scan_tab = tab_container_->addTab("🔍 Market Scan", false);
    scan_tab->add(scan_panel_);
}

void GuiApp::setupHistoryTab() {
    history_panel_ = tgui::Panel::create();
    history_panel_->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
    
    // History controls
    auto controls_panel = tgui::Panel::create();
    controls_panel->setPosition("2%", "2%");
    controls_panel->setSize("96%", "8%");
    controls_panel->getRenderer()->setBackgroundColor(tgui::Color(40, 40, 40));
    
    auto date_label = tgui::Label::create("Date Filter:");
    date_label->setPosition("2%", "20%");
    controls_panel->add(date_label);
    
    date_filter_ = tgui::ComboBox::create();
    date_filter_->setPosition("15%", "15%");
    date_filter_->setSize("20%", "70%");
    date_filter_->addItem("All Dates");
    date_filter_->addItem("Today");
    date_filter_->addItem("Yesterday");
    date_filter_->addItem("Last 7 Days");
    controls_panel->add(date_filter_);
    
    auto strategy_label = tgui::Label::create("Strategy:");
    strategy_label->setPosition("40%", "20%");
    controls_panel->add(strategy_label);
    
    strategy_filter_ = tgui::ComboBox::create();
    strategy_filter_->setPosition("53%", "15%");
    strategy_filter_->setSize("20%", "70%");
    strategy_filter_->addItem("All Strategies");
    strategy_filter_->addItem("ORB");
    strategy_filter_->addItem("VWAP");
    strategy_filter_->addItem("RSI");
    strategy_filter_->addItem("Breakout");
    controls_panel->add(strategy_filter_);
    
    history_panel_->add(controls_panel);
    
    // History list
    history_list_ = tgui::ListView::create();
    history_list_->setPosition("2%", "12%");
    history_list_->setSize("96%", "86%");
    history_list_->addColumn("Time", 120);
    history_list_->addColumn("Symbol", 80);
    history_list_->addColumn("Strategy", 100);
    history_list_->addColumn("Type", 80);
    history_list_->addColumn("Entry", 80);
    history_list_->addColumn("Target", 80);
    history_list_->addColumn("Confidence", 80);
    history_list_->addColumn("Result", 80);
    
    history_panel_->add(history_list_);
    
    auto history_tab = tab_container_->addTab("📋 History", false);
    history_tab->add(history_panel_);
}

void GuiApp::setupStatusBar() {
    status_panel_ = tgui::Panel::create();
    status_panel_->setPosition(0, "100% - 50");
    status_panel_->setSize("100%", 50);
    status_panel_->getRenderer()->setBackgroundColor(tgui::Color(20, 20, 20));
    status_panel_->getRenderer()->setBorderColor(tgui::Color::White);
    status_panel_->getRenderer()->setBorders({0, 1, 0, 0});
    
    market_status_label_ = tgui::Label::create("Market Status: Loading...");
    market_status_label_->setPosition("2%", "20%");
    market_status_label_->setTextSize(12);
    status_panel_->add(market_status_label_);
    
    last_update_label_ = tgui::Label::create("Last Update: Never");
    last_update_label_->setPosition("30%", "20%");
    last_update_label_->setTextSize(12);
    status_panel_->add(last_update_label_);
    
    refresh_button_ = tgui::Button::create("🔄 Refresh");
    refresh_button_->setPosition("60%", "10%");
    refresh_button_->setSize(80, 30);
    refresh_button_->onPress([this]() { onRefreshClicked(); });
    status_panel_->add(refresh_button_);
    
    auto_refresh_button_ = tgui::Button::create("⏰ Auto: ON");
    auto_refresh_button_->setPosition("75%", "10%");
    auto_refresh_button_->setSize(80, 30);
    auto_refresh_button_->onPress([this]() { onAutoRefreshToggled(); });
    status_panel_->add(auto_refresh_button_);
    
    update_progress_ = tgui::ProgressBar::create();
    update_progress_->setPosition("85%", "20%");
    update_progress_->setSize(100, 10);
    update_progress_->setMinimum(0);
    update_progress_->setMaximum(100);
    update_progress_->setValue(0);
    status_panel_->add(update_progress_);
    
    gui_->add(status_panel_);
}

void GuiApp::run() {
    std::cout << "🚀 Starting main application loop..." << std::endl;
    running_ = true;
    
    // Add a simple label to test the window
    auto testLabel = tgui::Label::create("🎯 HFT Trading System Loading...");
    testLabel->setPosition(50, 50);
    testLabel->setTextSize(18);
    gui_->add(testLabel);
    
    std::cout << "💻 Window should be visible now! Look for 'HFT Trading System' window" << std::endl;
    
    while (window_.isOpen() && running_) {
        while (auto event = window_.pollEvent()) {
            gui_->handleEvent(*event);
            
            if (event->is<sf::Event::Closed>()) {
                onWindowClosed();
            }
        }
        
        // Update GUI periodically
        static sf::Clock gui_update_clock;
        if (gui_update_clock.getElapsedTime().asSeconds() > 1.0f) {
            updateStatusBar();
            updateMarketStatus();
            updateLiveDataTable();  // Add this to update the stock table
            gui_update_clock.restart();
        }
        
        window_.clear(sf::Color(15, 15, 15));
        gui_->draw();
        window_.display();
        
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }
}

void GuiApp::cleanup() {
    running_ = false;
    stopDataUpdateThread();
    
    if (history_manager_) {
        history_manager_->cleanup();
    }
}

void GuiApp::startDataUpdateThread() {
    if (data_update_active_) return;
    
    data_update_active_ = true;
    data_update_thread_ = std::thread([this]() {
        while (data_update_active_) {
            try {
                updateMarketData();
                updateTradingSetups();
                updateMarketScan();
                
                std::this_thread::sleep_for(std::chrono::seconds(config_.update_interval_seconds));
            } catch (const std::exception& e) {
                std::cerr << "Data update error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    });
}

void GuiApp::stopDataUpdateThread() {
    data_update_active_ = false;
    if (data_update_thread_.joinable()) {
        data_update_thread_.join();
    }
}

void GuiApp::updateMarketData() {
    if (!api_) return;
    
    current_stocks_ = api_->getAllNifty50Data();
    last_update_ = std::chrono::system_clock::now();
    
    // Update GUI on main thread
    updateLiveDataTable();
}

void GuiApp::updateTradingSetups() {
    if (!strategy_engine_ || current_stocks_.empty()) return;
    
    current_setups_.clear();
    
    for (const auto& [symbol, stock] : current_stocks_) {
        auto historical = api_->getHistoricalData(symbol, "1mo");
        auto signals = strategy_engine_->analyzeStock(stock, historical);
        
        if (!signals.empty()) {
            auto setup = strategy_engine_->combineSignalsToSetup(symbol, signals);
            if (setup.overall_confidence > 0.5) {
                current_setups_.push_back(setup);
            }
        }
    }
    
    updateSetupsPanel();
}

void GuiApp::updateMarketScan() {
    if (!api_) return;
    
    current_scan_ = api_->getMarketScan();
    updateMarketScanLists();
}

void GuiApp::updateLiveDataTable() {
    if (!stocks_table_) return;
    
    stocks_table_->removeAllItems();
    
    for (const auto& [symbol, stock] : current_stocks_) {
        auto clean_symbol = symbol.substr(0, symbol.find('.'));
        
        std::vector<tgui::String> row = {
            clean_symbol,
            tgui::String(std::to_string((int)stock.current_price)),
            tgui::String(MarketDataUtils::formatPercent(stock.change, 2)),
            tgui::String(MarketDataUtils::formatPercent(stock.change_percent, 2)),
            tgui::String(MarketDataUtils::formatVolume(stock.volume)),
            tgui::String(std::to_string((int)stock.rsi_14)),
            stock.is_breakout ? "🚀 BUY" : (stock.is_breakdown ? "📉 SELL" : "➖ HOLD")
        };
        
        stocks_table_->addItem(row);
    }
}

void GuiApp::updateSetupsPanel() {
    if (!setups_list_) return;
    
    setups_list_->removeAllItems();
    
    // Sort setups by confidence
    auto sorted_setups = current_setups_;
    std::sort(sorted_setups.begin(), sorted_setups.end(),
             [](const TradingSetup& a, const TradingSetup& b) {
                 return a.overall_confidence > b.overall_confidence;
             });
    
    int count = 0;
    for (const auto& setup : sorted_setups) {
        if (count++ >= MAX_SETUPS_DISPLAY) break;
        
        auto clean_symbol = setup.symbol.substr(0, setup.symbol.find('.'));
        
        std::vector<tgui::String> row = {
            clean_symbol,
            SignalUtils::signalTypeToString(setup.primary_signal),
            tgui::String(std::to_string((int)setup.recommended_entry)),
            tgui::String(std::to_string(setup.risk_reward_ratio).substr(0, 4)),
            tgui::String(std::to_string((int)(setup.overall_confidence * 100)) + "%")
        };
        
        setups_list_->addItem(row);
    }
}

void GuiApp::updateMarketScanLists() {
    // Update gainers list
    if (gainers_list_) {
        gainers_list_->removeAllItems();
        for (const auto& stock : current_scan_.top_gainers) {
            auto clean_symbol = stock.symbol.substr(0, stock.symbol.find('.'));
            std::vector<tgui::String> row = {
                clean_symbol,
                MarketDataUtils::formatPercent(stock.change_percent, 2),
                tgui::String(std::to_string((int)stock.current_price))
            };
            gainers_list_->addItem(row);
        }
    }
    
    // Update losers list
    if (losers_list_) {
        losers_list_->removeAllItems();
        for (const auto& stock : current_scan_.top_losers) {
            auto clean_symbol = stock.symbol.substr(0, stock.symbol.find('.'));
            std::vector<tgui::String> row = {
                clean_symbol,
                MarketDataUtils::formatPercent(stock.change_percent, 2),
                tgui::String(std::to_string((int)stock.current_price))
            };
            losers_list_->addItem(row);
        }
    }
    
    // Update high volume list
    if (high_volume_list_) {
        high_volume_list_->removeAllItems();
        for (const auto& stock : current_scan_.high_volume) {
            auto clean_symbol = stock.symbol.substr(0, stock.symbol.find('.'));
            std::vector<tgui::String> row = {
                clean_symbol,
                MarketDataUtils::formatVolume(stock.volume),
                tgui::String(std::to_string(stock.volume_ratio).substr(0, 4))
            };
            high_volume_list_->addItem(row);
        }
    }
}

void GuiApp::updateStatusBar() {
    if (last_update_label_) {
        std::string update_text = "Last Update: " + formatDateTime(last_update_);
        last_update_label_->setText(update_text);
    }
}

void GuiApp::updateMarketStatus() {
    if (market_status_label_) {
        auto market_info = MarketDataUtils::getCurrentMarketStatus();
        std::string status_text = "Market Status: " + market_info.status_message;
        market_status_label_->setText(status_text);
        
        // Color code the status
        if (market_info.status == MarketStatus::OPEN) {
            market_status_label_->getRenderer()->setTextColor(tgui::Color::Green);
        } else {
            market_status_label_->getRenderer()->setTextColor(tgui::Color::Red);
        }
    }
}

// Event handlers (stub implementations)
void GuiApp::onWindowClosed() { running_ = false; }
void GuiApp::onRefreshClicked() { /* Trigger immediate data refresh */ }
void GuiApp::onAutoRefreshToggled() { /* Toggle auto refresh */ }
void GuiApp::onStockSelected() { /* Show stock details */ }
void GuiApp::onSetupSelected() { /* Show setup details */ }
void GuiApp::onTabChanged() { /* Handle tab change */ }
void GuiApp::onMenuItemClicked(const std::string& item) { (void)item; }
void GuiApp::onExportHistory() { /* Export history to CSV */ }
void GuiApp::onSettingsClicked() { /* Show settings dialog */ }

// Utility methods (stub implementations)
void GuiApp::showNotification(const std::string& title, const std::string& message) { 
    std::cout << "🔔 " << title << ": " << message << std::endl;
}

void GuiApp::playAlertSound() { /* Play alert sound */ }

tgui::Color GuiApp::getColorForChange(double change_percent) {
    return change_percent > 0 ? tgui::Color::Green : tgui::Color::Red;
}

std::string GuiApp::formatDateTime(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

void GuiApp::centerWindow() {
    try {
        auto desktop = sf::VideoMode::getDesktopMode();
        window_.setPosition(sf::Vector2i(
            static_cast<int>((desktop.size.x - config_.window_width) / 2),
            static_cast<int>((desktop.size.y - config_.window_height) / 2)
        ));
    } catch (const std::exception& e) {
        std::cout << "  ⚠️ Window centering failed (running headless?): " << e.what() << std::endl;
        // Continue without centering
    }
}

void GuiApp::loadTheme(const std::string& theme_name) {
    if (theme_name == "Dark") {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
}

void GuiApp::applyDarkTheme() {
    // Apply dark theme to all GUI components
    // Note: TGUI 1.x uses themes instead of global text colors
}

void GuiApp::applyLightTheme() {
    // Apply light theme to all GUI components
    // Note: TGUI 1.x uses themes instead of global text colors
}

void GuiApp::saveWindowState() { /* Save window position/size */ }
void GuiApp::loadWindowState() { /* Load window position/size */ }
void GuiApp::handleError(const std::string& error_message) { 
    std::cerr << "❌ " << error_message << std::endl;
}
void GuiApp::showErrorDialog(const std::string& title, const std::string& message) {
    std::cerr << title << ": " << message << std::endl;
}
