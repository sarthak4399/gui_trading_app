#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Creating SFML window test..." << std::endl;
    
    // Create a simple window
    sf::RenderWindow window(sf::VideoMode(400, 300), "SFML Window Test");
    window.setFramerateLimit(60);
    
    if (!window.isOpen()) {
        std::cout << "❌ Failed to create window!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Window created successfully!" << std::endl;
    std::cout << "Window should be visible now. Press ESC to close or wait 10 seconds." << std::endl;
    
    sf::Clock clock;
    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    shape.setPosition(175, 125);
    
    while (window.isOpen() && clock.getElapsedTime().asSeconds() < 10) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close();
            }
        }
        
        window.clear(sf::Color::Black);
        window.draw(shape);
        window.display();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "Window test completed." << std::endl;
    return 0;
}
