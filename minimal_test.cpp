#include "gui_app.h"
#include <iostream>

// Minimal test version of the GUI app
int main() {
    try {
        std::cout << "🧪 Testing minimal HFT app window..." << std::endl;
        
        GuiApp app;
        std::cout << "• GuiApp object created" << std::endl;
        
        // Try to call just setupWindow
        std::cout << "• Attempting window setup..." << std::endl;
        
        sf::RenderWindow window;
        sf::VideoMode videoMode({1600u, 1200u});
        window.create(videoMode, "Test HFT Window", sf::Style::Default);
        
        if (!window.isOpen()) {
            std::cout << "❌ Window creation failed" << std::endl;
            return -1;
        }
        
        std::cout << "✅ Window created successfully!" << std::endl;
        
        tgui::Gui gui{window};
        std::cout << "✅ TGUI attached to window!" << std::endl;
        
        window.close();
        std::cout << "✅ Minimal HFT app test successful!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Test failed: " << e.what() << std::endl;
        return -1;
    }
}
