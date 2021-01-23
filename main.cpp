#include <iostream>

#include <SFML/Graphics.hpp>

#include <map.hpp>

int main() {

  sf::RenderWindow window(sf::VideoMode(840, 600), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);

  TiledMap map(std::string("assets/map.json"));

  while (window.isOpen()) {

    sf::Event event;
    
    while (window.pollEvent(event)) {
      switch (event.type) {
      case sf::Event::Closed:
        window.close();
        break;
      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Escape) {
          window.close();
        }
        break;
      }
    }

    window.clear(sf::Color::Black);
    window.draw(map);
    window.display();
  }

  return 0;
}