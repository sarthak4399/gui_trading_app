#include "simple_hft_gui.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

SimpleHFTGui::SimpleHFTGui() : running_(false), data_ready_(false) {}

SimpleHFTGui::~SimpleHFTGui() {
    shutdown();
}

bool SimpleHFTGui::initialize() {
    std::cout << "🚀 Initializing Simple HFT GUI..." << std::endl;
    
    // Initialize API
    api_ = std::make_unique<YahooFinanceAPI>();
    
    // Setup window and UI
    setupWindow();
    setupUI();
    setupDataThread();
    
    std::cout << "✅ Simple HFT GUI initialized successfully!" << std::endl;
    return true;
}

void SimpleHFTGui::setupWindow() {
    window_.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "HFT Trading System - Live Market Data");
    window_.setFramerateLimit(60);
    
    gui_ = std::make_unique<tgui::Gui>(window_);
}

void SimpleHFTGui::setupUI() {
    // Main panel with dark theme
    main_panel_ = tgui::Panel::create();
    main_panel_->setSize("100%", "100%");
    main_panel_->getRenderer()->setBackgroundColor(tgui::Color(25, 25, 25));
    gui_->add(main_panel_);
    
    // Title bar
    auto title = tgui::Label::create("🚀 HFT TRADING SYSTEM - LIVE NIFTY 50 DATA");
    title->setPosition("2%", "1%");
    title->setTextSize(20);
    title->getRenderer()->setTextColor(tgui::Color::White);
    main_panel_->add(title);
    
    // Control panel
    auto control_panel = tgui::Panel::create();
    control_panel->setPosition("2%", "6%");
    control_panel->setSize("96%", "8%");
    control_panel->getRenderer()->setBackgroundColor(tgui::Color(35, 35, 35));
    control_panel->getRenderer()->setBorderColor(tgui::Color(60, 60, 60));
    control_panel->getRenderer()->setBorders(1);
    main_panel_->add(control_panel);
    
    // Status label
    status_label_ = tgui::Label::create("Status: Initializing...");
    status_label_->setPosition("2%", "25%");
    status_label_->setTextSize(14);
    status_label_->getRenderer()->setTextColor(tgui::Color::Yellow);
    control_panel->add(status_label_);
    
    // Time label
    time_label_ = tgui::Label::create("Last Update: Never");
    time_label_->setPosition("35%", "25%");
    time_label_->setTextSize(14);
    time_label_->getRenderer()->setTextColor(tgui::Color::Cyan);
    control_panel->add(time_label_);
    
    // Count label
    count_label_ = tgui::Label::create("Stocks: 0/50");
    count_label_->setPosition("65%", "25%");
    count_label_->setTextSize(14);
    count_label_->getRenderer()->setTextColor(tgui::Color::Green);
    control_panel->add(count_label_);
    
    // Refresh button
    refresh_btn_ = tgui::Button::create("🔄 Refresh");
    refresh_btn_->setPosition("2%", "60%");
    refresh_btn_->setSize("15%", "35%");
    refresh_btn_->getRenderer()->setBackgroundColor(tgui::Color(0, 120, 215));
    refresh_btn_->getRenderer()->setTextColor(tgui::Color::White);
    refresh_btn_->onPress([this] { onRefreshClicked(); });
    control_panel->add(refresh_btn_);
    
    // Sort dropdown
    auto sort_label = tgui::Label::create("Sort by:");
    sort_label->setPosition("20%", "60%");
    sort_label->setTextSize(12);
    sort_label->getRenderer()->setTextColor(tgui::Color::White);
    control_panel->add(sort_label);
    
    sort_combo_ = tgui::ComboBox::create();
    sort_combo_->setPosition("28%", "60%");
    sort_combo_->setSize("20%", "35%");
    sort_combo_->addItem("Symbol");
    sort_combo_->addItem("Price (High to Low)");
    sort_combo_->addItem("Change % (High to Low)");
    sort_combo_->addItem("Change % (Low to High)");
    sort_combo_->addItem("Volume");
    sort_combo_->setSelectedItem("Symbol");
    sort_combo_->onItemSelect([this] { onSortChanged(); });
    control_panel->add(sort_combo_);
    
    // Stock list with improved styling
    stock_list_ = tgui::ListView::create();
    stock_list_->setPosition("2%", "16%");
    stock_list_->setSize("96%", "82%");
    stock_list_->getRenderer()->setBackgroundColor(tgui::Color(30, 30, 30));
    stock_list_->getRenderer()->setTextColor(tgui::Color::White);
    stock_list_->getRenderer()->setHeaderBackgroundColor(tgui::Color(45, 45, 45));
    stock_list_->getRenderer()->setHeaderTextColor(tgui::Color::White);
    stock_list_->getRenderer()->setSelectedBackgroundColor(tgui::Color(0, 120, 215, 100));
    stock_list_->getRenderer()->setBorderColor(tgui::Color(60, 60, 60));
    stock_list_->getRenderer()->setBorders(1);
    
    // Add columns with proper widths
    stock_list_->addColumn("Symbol", 100);
    stock_list_->addColumn("Price (₹)", 120);
    stock_list_->addColumn("Change", 100);
    stock_list_->addColumn("Change %", 100);
    stock_list_->addColumn("Volume", 120);
    stock_list_->addColumn("RSI", 80);
    stock_list_->addColumn("Signal", 100);
    stock_list_->addColumn("Status", 100);
    
    main_panel_->add(stock_list_);
}

void SimpleHFTGui::setupDataThread() {
    running_ = true;
    data_thread_ = std::thread(&SimpleHFTGui::dataUpdateLoop, this);
}

void SimpleHFTGui::dataUpdateLoop() {
    while (running_) {
        try {
            std::cout << "📡 Fetching market data..." << std::endl;
            current_stocks_ = api_->getAllNifty50Data();
            last_update_ = std::chrono::system_clock::now();
            data_ready_ = true;
            std::cout << "✅ Fetched data for " << current_stocks_.size() << " stocks" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "❌ Error fetching data: " << e.what() << std::endl;
        }
        
        // Wait for next update
        for (int i = 0; i < UPDATE_INTERVAL * 10 && running_; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void SimpleHFTGui::updateStockList() {
    if (!data_ready_ || current_stocks_.empty()) return;
    
    stock_list_->removeAllItems();
    
    // Convert to vector for sorting
    std::vector<std::pair<std::string, StockData>> stock_vec(
        current_stocks_.begin(), current_stocks_.end());
    
    // Apply current sort
    std::string sort_criteria = sort_combo_->getSelectedItem().toStdString();
    if (sort_criteria == "Price (High to Low)") {
        std::sort(stock_vec.begin(), stock_vec.end(),
            [](const auto& a, const auto& b) {
                return a.second.current_price > b.second.current_price;
            });
    } else if (sort_criteria == "Change % (High to Low)") {
        std::sort(stock_vec.begin(), stock_vec.end(),
            [](const auto& a, const auto& b) {
                return a.second.change_percent > b.second.change_percent;
            });
    } else if (sort_criteria == "Change % (Low to High)") {
        std::sort(stock_vec.begin(), stock_vec.end(),
            [](const auto& a, const auto& b) {
                return a.second.change_percent < b.second.change_percent;
            });
    } else if (sort_criteria == "Volume") {
        std::sort(stock_vec.begin(), stock_vec.end(),
            [](const auto& a, const auto& b) {
                return a.second.volume > b.second.volume;
            });
    }
    
    // Add rows
    for (const auto& [symbol, stock] : stock_vec) {
        std::string clean_symbol = symbol.substr(0, symbol.find('.'));
        
        // Determine status
        std::string status = "Active";
        if (stock.current_price <= 0) status = "No Data";
        else if (stock.volume == 0) status = "Low Vol";
        
        // Create row
        std::vector<tgui::String> row = {
            clean_symbol,
            formatPrice(stock.current_price),
            (stock.change >= 0 ? "+" : "") + formatPrice(stock.change),
            (stock.change_percent >= 0 ? "+" : "") + formatPercent(stock.change_percent),
            std::to_string((long long)stock.volume),
            std::to_string((int)stock.rsi_14),
            stock.is_breakout ? "🚀 BUY" : (stock.is_breakdown ? "📉 SELL" : "⏸️ HOLD"),
            status
        };
        
        stock_list_->addItem(row);
        
        // Set row color based on change
        int item_index = stock_list_->getItemCount() - 1;
        if (stock.change > 0) {
            stock_list_->setItemColor(item_index, tgui::Color(0, 255, 0, 50));
        } else if (stock.change < 0) {
            stock_list_->setItemColor(item_index, tgui::Color(255, 0, 0, 50));
        }
    }
}

void SimpleHFTGui::updateStatusInfo() {
    // Update status
    if (data_ready_) {
        status_label_->setText("Status: ✅ Live Data Active");
        status_label_->getRenderer()->setTextColor(tgui::Color::Green);
    } else {
        status_label_->setText("Status: ⏳ Loading...");
        status_label_->getRenderer()->setTextColor(tgui::Color::Yellow);
    }
    
    // Update time
    if (data_ready_) {
        auto time_t = std::chrono::system_clock::to_time_t(last_update_);
        std::stringstream ss;
        ss << "Last Update: " << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        time_label_->setText(ss.str());
    }
    
    // Update count
    count_label_->setText("Stocks: " + std::to_string(current_stocks_.size()) + "/50");
    if (current_stocks_.size() == 50) {
        count_label_->getRenderer()->setTextColor(tgui::Color::Green);
    } else {
        count_label_->getRenderer()->setTextColor(tgui::Color::Yellow);
    }
}

void SimpleHFTGui::onRefreshClicked() {
    std::cout << "🔄 Manual refresh requested" << std::endl;
    data_ready_ = false;
    status_label_->setText("Status: 🔄 Refreshing...");
    status_label_->getRenderer()->setTextColor(tgui::Color::Yellow);
}

void SimpleHFTGui::onSortChanged() {
    updateStockList();
}

std::string SimpleHFTGui::formatPrice(double price) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << price;
    return ss.str();
}

std::string SimpleHFTGui::formatPercent(double percent) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << percent << "%";
    return ss.str();
}

tgui::Color SimpleHFTGui::getChangeColor(double change) {
    if (change > 0) return tgui::Color::Green;
    if (change < 0) return tgui::Color::Red;
    return tgui::Color::White;
}

void SimpleHFTGui::run() {
    std::cout << "🚀 Starting Simple HFT GUI..." << std::endl;
    
    sf::Clock update_clock;
    
    while (window_.isOpen() && running_) {
        // Handle events
        while (auto event = window_.pollEvent()) {
            gui_->handleEvent(*event);
            
            if (event->is<sf::Event::Closed>()) {
                running_ = false;
            }
            
            if (event->is<sf::Event::KeyPressed>()) {
                auto key_event = event->getIf<sf::Event::KeyPressed>();
                if (key_event->code == sf::Keyboard::F5) {
                    onRefreshClicked();
                } else if (key_event->code == sf::Keyboard::Escape) {
                    running_ = false;
                }
            }
        }
        
        // Update UI every second
        if (update_clock.getElapsedTime().asSeconds() >= 1.0f) {
            updateStockList();
            updateStatusInfo();
            update_clock.restart();
        }
        
        // Render
        window_.clear(sf::Color(15, 15, 15));
        gui_->draw();
        window_.display();
        
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }
    
    shutdown();
}

void SimpleHFTGui::shutdown() {
    running_ = false;
    
    if (data_thread_.joinable()) {
        data_thread_.join();
    }
    
    std::cout << "✅ Simple HFT GUI shutdown complete" << std::endl;
}
