#pragma once

#include "market_data.h"
#include "trading_signal.h"
#include "yahoo_finance_api.h"
#include "strategy_engine.h"
#include "history_manager.h"

#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

/**
 * Main GUI Application for HFT Trading System
 */
class GuiApp {
public:
    GuiApp();
    ~GuiApp();
    
    // Application lifecycle
    bool initialize();
    void run();
    void cleanup();
    
private:
    // Core components
    sf::RenderWindow window_;
    std::unique_ptr<tgui::Gui> gui_;
    
    std::unique_ptr<YahooFinanceAPI> api_;
    std::unique_ptr<StrategyEngine> strategy_engine_;
    std::unique_ptr<HistoryManager> history_manager_;
    
    // Data management
    std::map<std::string, StockData> current_stocks_;
    std::map<std::string, HistoricalData> historical_data_;
    MarketScan current_scan_;
    std::vector<TradingSetup> current_setups_;
    
    // Threading and updates
    std::atomic<bool> running_;
    std::atomic<bool> data_update_active_;
    std::thread data_update_thread_;
    std::chrono::system_clock::time_point last_update_;
    
    // GUI Components
    tgui::Panel::Ptr main_panel_;
    tgui::MenuBar::Ptr menu_bar_;
    tgui::TabContainer::Ptr tab_container_;
    
    // Live Data Tab
    tgui::Panel::Ptr live_panel_;
    tgui::ListView::Ptr stocks_table_;
    tgui::Panel::Ptr setups_panel_;
    tgui::ListView::Ptr setups_list_;
    
    // Market Scan Tab
    tgui::Panel::Ptr scan_panel_;
    tgui::ListView::Ptr gainers_list_;
    tgui::ListView::Ptr losers_list_;
    tgui::ListView::Ptr high_volume_list_;
    
    // History Tab
    tgui::Panel::Ptr history_panel_;
    tgui::ListView::Ptr history_list_;
    tgui::ComboBox::Ptr date_filter_;
    tgui::ComboBox::Ptr strategy_filter_;
    
    // Status and Controls
    tgui::Panel::Ptr status_panel_;
    tgui::Label::Ptr market_status_label_;
    tgui::Label::Ptr last_update_label_;
    tgui::ProgressBar::Ptr update_progress_;
    tgui::Button::Ptr refresh_button_;
    tgui::Button::Ptr auto_refresh_button_;
    
    // Configuration
    struct Config {
        int window_width = 1600;
        int window_height = 1200;
        int update_interval_seconds = 5;
        bool auto_refresh_enabled = true;
        double min_confidence_threshold = 0.6;
        bool sound_alerts = true;
        std::string theme_name = "Dark";
    } config_;
    
    // Initialization methods
    void setupWindow();
    void setupGUI();
    void setupMenuBar();
    void setupMainTabs();
    void setupLiveDataTab();
    void setupMarketScanTab();
    void setupHistoryTab();
    void setupStatusBar();
    
    // Data update methods
    void startDataUpdateThread();
    void stopDataUpdateThread();
    void dataUpdateLoop();
    void updateMarketData();
    void updateTradingSetups();
    void updateMarketScan();
    
    // GUI update methods
    void updateLiveDataTable();
    void updateSetupsPanel();
    void updateMarketScanLists();
    void updateHistoryList();
    void updateStatusBar();
    void updateMarketStatus();
    
    // Event handlers
    void onWindowClosed();
    void onRefreshClicked();
    void onAutoRefreshToggled();
    void onStockSelected();
    void onSetupSelected();
    void onTabChanged();
    void onMenuItemClicked(const std::string& item);
    void onExportHistory();
    void onSettingsClicked();
    
    // Utility methods
    void showNotification(const std::string& title, const std::string& message);
    void playAlertSound();
    tgui::Color getColorForChange(double change_percent);
    std::string formatDateTime(const std::chrono::system_clock::time_point& time);
    void centerWindow();
    void saveWindowState();
    void loadWindowState();
    
    // Theme management
    void loadTheme(const std::string& theme_name);
    void applyDarkTheme();
    void applyLightTheme();
    
    // Error handling
    void handleError(const std::string& error_message);
    void showErrorDialog(const std::string& title, const std::string& message);
    
    // Constants
    static const int MIN_WINDOW_WIDTH = 1200;
    static const int MIN_WINDOW_HEIGHT = 800;
    static const int MAX_SETUPS_DISPLAY = 20;
    static const int MAX_HISTORY_ITEMS = 1000;
};

/**
 * Stock Detail Dialog for showing comprehensive stock information
 */
class StockDetailDialog {
public:
    StockDetailDialog(tgui::Gui& gui, const StockData& stock, 
                     const HistoricalData& historical);
    
    void show();
    void hide();
    
private:
    tgui::Gui& gui_;
    StockData stock_data_;
    HistoricalData historical_data_;
    
    tgui::ChildWindow::Ptr window_;
    tgui::Panel::Ptr content_panel_;
    tgui::ListView::Ptr indicators_list_;
    tgui::Panel::Ptr chart_panel_;
    
    void setupDialog();
    void populateIndicators();
    void drawSimpleChart();
};

/**
 * Settings Dialog for application configuration
 */
class SettingsDialog {
public:
    SettingsDialog(tgui::Gui& gui);
    
    void show();
    void hide();
    
private:
    tgui::Gui& gui_;
    
    tgui::ChildWindow::Ptr window_;
    tgui::Slider::Ptr update_interval_slider_;
    tgui::CheckBox::Ptr auto_refresh_checkbox_;
    tgui::CheckBox::Ptr sound_alerts_checkbox_;
    tgui::ComboBox::Ptr theme_combo_;
    tgui::Button::Ptr save_button_;
    tgui::Button::Ptr cancel_button_;
    
    void setupDialog();
    void loadCurrentSettings();
    void onSaveClicked();
    void onCancelClicked();
};
