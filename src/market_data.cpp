#include "market_data.h"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace MarketDataUtils {
    
    std::vector<std::string> getNifty50Symbols() {
        return {
            "RELIANCE.NS", "TCS.NS", "INFY.NS", "HDFC.NS", "HDFCBANK.NS",
            "ICICIBANK.NS", "KOTAKBANK.NS", "HINDUNILVR.NS", "LT.NS", "SBIN.NS",
            "BHARTIARTL.NS", "ASIANPAINT.NS", "ITC.NS", "AXISBANK.NS", "MARUTI.NS",
            "BAJFINANCE.NS", "NESTLEIND.NS", "ULTRACEMCO.NS", "TITAN.NS", "WIPRO.NS",
            "M&M.NS", "SUNPHARMA.NS", "TECHM.NS", "NTPC.NS", "POWERGRID.NS",
            "BAJAJFINSV.NS", "HCLTECH.NS", "DIVISLAB.NS", "ADANIENT.NS", "TATAMOTORS.NS",
            "INDUSINDBK.NS", "COALINDIA.NS", "TATASTEEL.NS", "GRASIM.NS", "CIPLA.NS",
            "DRREDDY.NS", "EICHERMOT.NS", "APOLLOHOSP.NS", "BRITANNIA.NS", "BPCL.NS",
            "TATACONSUM.NS", "ADANIPORTS.NS", "JSWSTEEL.NS", "HEROMOTOCO.NS", "UPL.NS",
            "BAJAJ-AUTO.NS", "SBILIFE.NS", "ONGC.NS", "HINDALCO.NS", "IOC.NS"
        };
    }
    
    std::string formatPrice(double price, int decimals) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(decimals) << price;
        return oss.str();
    }
    
    std::string formatVolume(long long volume) {
        if (volume >= 10000000) { // 1 Crore
            return formatPrice(volume / 10000000.0, 2) + "Cr";
        } else if (volume >= 100000) { // 1 Lakh
            return formatPrice(volume / 100000.0, 2) + "L";
        } else if (volume >= 1000) { // 1 Thousand
            return formatPrice(volume / 1000.0, 1) + "K";
        }
        return std::to_string(volume);
    }
    
    std::string formatPercent(double percent, int decimals) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(decimals);
        if (percent > 0) oss << "+";
        oss << percent << "%";
        return oss.str();
    }
    
    MarketInfo getCurrentMarketStatus() {
        MarketInfo info;
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        // Indian market hours: 9:15 AM to 3:30 PM IST
        int current_minutes = tm.tm_hour * 60 + tm.tm_min;
        int market_open = 9 * 60 + 15;   // 9:15 AM
        int market_close = 15 * 60 + 30; // 3:30 PM
        int pre_market_start = 9 * 60;   // 9:00 AM
        
        // Check if it's a weekend
        if (tm.tm_wday == 0 || tm.tm_wday == 6) { // Sunday or Saturday
            info.status = MarketStatus::CLOSED;
            info.is_trading_day = false;
            info.status_message = "Weekend - Market Closed";
            return info;
        }
        
        if (current_minutes < pre_market_start) {
            info.status = MarketStatus::CLOSED;
            info.status_message = "Market Closed";
        } else if (current_minutes < market_open) {
            info.status = MarketStatus::PRE_MARKET;
            info.status_message = "Pre-Market Session";
        } else if (current_minutes <= market_close) {
            info.status = MarketStatus::OPEN;
            info.status_message = "Market Open";
        } else {
            info.status = MarketStatus::CLOSED;
            info.status_message = "Market Closed";
        }
        
        return info;
    }
    
    bool isTradingTime() {
        auto info = getCurrentMarketStatus();
        return info.status == MarketStatus::OPEN && info.is_trading_day;
    }
    
    std::chrono::minutes getTimeToMarketEvent() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        int current_minutes = tm.tm_hour * 60 + tm.tm_min;
        int market_open = 9 * 60 + 15;   // 9:15 AM
        int market_close = 15 * 60 + 30; // 3:30 PM
        
        if (current_minutes < market_open) {
            return std::chrono::minutes(market_open - current_minutes);
        } else if (current_minutes <= market_close) {
            return std::chrono::minutes(market_close - current_minutes);
        } else {
            // Time until next day market open
            int minutes_until_next_day = (24 * 60) - current_minutes + market_open;
            return std::chrono::minutes(minutes_until_next_day);
        }
    }
}
