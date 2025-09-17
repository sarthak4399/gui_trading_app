#include <curl/curl.h>
#include <iostream>
#include <string>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test different Yahoo Finance endpoints
    std::vector<std::string> test_urls = {
        "https://query1.finance.yahoo.com/v7/finance/quote?symbols=AAPL",
        "https://query2.finance.yahoo.com/v7/finance/quote?symbols=AAPL", 
        "https://finance.yahoo.com/quote/AAPL/",
        "https://query1.finance.yahoo.com/v8/finance/chart/AAPL",
        "https://query1.finance.yahoo.com/v10/finance/quoteSummary/AAPL?modules=price"
    };
    
    for (const auto& url : test_urls) {
        std::cout << "\n🧪 Testing: " << url << std::endl;
        
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
            
            if (res == CURLE_OK) {
                std::cout << "✅ HTTP " << response_code << " - Response length: " << response.length() << std::endl;
                if (response_code == 200 && !response.empty()) {
                    std::cout << "📊 Sample data: " << response.substr(0, 200) << "..." << std::endl;
                }
            } else {
                std::cout << "❌ CURL Error: " << curl_easy_strerror(res) << std::endl;
            }
            
            curl_easy_cleanup(curl);
        }
    }
    
    curl_global_cleanup();
    return 0;
}
