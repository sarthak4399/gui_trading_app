#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    std::cout << "🧪 Testing basic SFML window creation..." << std::endl;
    
    try {
        std::cout << "• Creating 800x600 test window..." << std::endl;
        sf::RenderWindow window(sf::VideoMode({800u, 600u}), "SFML Test Window");
        
        if (!window.isOpen()) {
            std::cout << "❌ Window creation failed" << std::endl;
            return -1;
        }
        
        std::cout << "✅ Window created successfully!" << std::endl;
        std::cout << "• Testing basic rendering..." << std::endl;
        
        // Simple test - just clear and display
        window.clear(sf::Color::Blue);
        window.display();
        
        std::cout << "✅ Basic rendering works!" << std::endl;
        std::cout << "• Closing window..." << std::endl;
        
        window.close();
        std::cout << "✅ SFML GUI test successful - display is working!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ SFML test failed: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cout << "❌ SFML test failed: Unknown error" << std::endl;
        return -1;
    }
}
