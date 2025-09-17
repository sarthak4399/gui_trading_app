#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include "yahoo_finance_api.h"

class SimpleHFTGui {
private:
    // Core components
    sf::RenderWindow window_;
    std::unique_ptr<tgui::Gui> gui_;
    std::unique_ptr<YahooFinanceAPI> api_;
    
    // GUI elements
    tgui::ListView::Ptr stock_list_;
    tgui::Label::Ptr status_label_;
    tgui::Label::Ptr time_label_;
    tgui::Label::Ptr count_label_;
    tgui::Button::Ptr refresh_btn_;
    tgui::ComboBox::Ptr sort_combo_;
    tgui::Panel::Ptr main_panel_;
    
    // Data and threading
    std::map<std::string, StockData> current_stocks_;
    std::thread data_thread_;
    std::atomic<bool> running_;
    std::atomic<bool> data_ready_;
    std::chrono::system_clock::time_point last_update_;
    
    // Configuration
    static constexpr int WINDOW_WIDTH = 1000;
    static constexpr int WINDOW_HEIGHT = 700;
    static constexpr int UPDATE_INTERVAL = 5; // seconds
    
public:
    SimpleHFTGui();
    ~SimpleHFTGui();
    
    bool initialize();
    void run();
    void shutdown();
    
private:
    void setupWindow();
    void setupUI();
    void setupDataThread();
    void updateStockList();
    void updateStatusInfo();
    void onRefreshClicked();
    void onSortChanged();
    void sortStocks(const std::string& criteria);
    void dataUpdateLoop();
    std::string formatPrice(double price);
    std::string formatPercent(double percent);
    tgui::Color getChangeColor(double change);
};
