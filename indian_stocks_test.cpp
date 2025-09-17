#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test Indian stock symbols from NSE
    std::vector<std::string> indian_symbols = {
        "RELIANCE.NS",  // Reliance Industries
        "TCS.NS",       // Tata Consultancy Services
        "INFY.NS",      // Infosys
        "HDFCBANK.NS",  // HDFC Bank
        "ICICIBANK.NS"  // ICICI Bank
    };
    
    for (const auto& symbol : indian_symbols) {
        std::cout << "\n🇮🇳 Testing Indian stock: " << symbol << std::endl;
        
        std::string url = "https://query1.finance.yahoo.com/v8/finance/chart/" + symbol + "?interval=1m&range=1d";
        
        CURL* curl = curl_easy_init();
        std::string response;
        
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36");
            
            CURLcode res = curl_easy_perform(curl);
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            
            if (res == CURLE_OK && response_code == 200) {
                std::cout << "✅ Success - Response length: " << response.length() << std::endl;
                
                // Quick check for expected JSON structure
                if (response.find("\"chart\"") != std::string::npos && 
                    response.find("\"meta\"") != std::string::npos) {
                    std::cout << "📊 Valid chart data received" << std::endl;
                    
                    // Extract price from meta if available
                    size_t price_pos = response.find("\"regularMarketPrice\":");
                    if (price_pos != std::string::npos) {
                        size_t start = price_pos + 21;
                        size_t end = response.find(",", start);
                        if (end == std::string::npos) end = response.find("}", start);
                        if (end != std::string::npos) {
                            std::string price_str = response.substr(start, end - start);
                            std::cout << "💰 Current Price: ₹" << price_str << std::endl;
                        }
                    }
                } else {
                    std::cout << "⚠️ Unexpected response format" << std::endl;
                }
            } else {
                std::cout << "❌ HTTP " << response_code << " - Error: " << curl_easy_strerror(res) << std::endl;
            }
            
            curl_easy_cleanup(curl);
        }
    }
    
    curl_global_cleanup();
    return 0;
}
