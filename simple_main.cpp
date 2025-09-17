#include "simple_hft_gui.h"
#include <iostream>

int main() {
    try {
        SimpleHFTGui app;
        
        if (!app.initialize()) {
            std::cerr << "❌ Failed to initialize HFT GUI" << std::endl;
            return -1;
        }
        
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
