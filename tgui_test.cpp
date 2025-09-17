#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "🧪 Testing SFML + TGUI window creation..." << std::endl;
    
    try {
        std::cout << "• Creating SFML window..." << std::endl;
        sf::RenderWindow window(sf::VideoMode({1200u, 800u}), "TGUI Test Window");
        
        if (!window.isOpen()) {
            std::cout << "❌ SFML window creation failed" << std::endl;
            return -1;
        }
        std::cout << "✅ SFML window created!" << std::endl;
        
        std::cout << "• Initializing TGUI..." << std::endl;
        tgui::Gui gui{window};
        std::cout << "✅ TGUI initialized!" << std::endl;
        
        std::cout << "• Creating test widgets..." << std::endl;
        auto label = tgui::Label::create("HFT Trading Test");
        label->setPosition(50, 50);
        label->setTextSize(24);
        gui.add(label);
        std::cout << "✅ TGUI widgets created!" << std::endl;
        
        std::cout << "• Testing rendering loop..." << std::endl;
        for (int i = 0; i < 3; ++i) {
            while (auto event = window.pollEvent()) {
                gui.handleEvent(*event);
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                    break;
                }
            }
            
            window.clear();
            gui.draw();
            window.display();
            
            std::cout << "  Frame " << (i+1) << " rendered successfully" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        window.close();
        std::cout << "✅ SFML + TGUI test successful!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Test failed: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cout << "❌ Test failed: Unknown error" << std::endl;
        return -1;
    }
}
