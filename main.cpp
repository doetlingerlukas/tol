#include <iostream>

#include <SFML/Graphics.hpp>

#include <map.hpp>

int main() {

  sf::RenderWindow window(sf::VideoMode(840, 600), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);

  // define the level with an array of tile indices
  const int level[] = {
      0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0,
      1, 1, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3,
      0, 1, 0, 0, 2, 0, 3, 3, 3, 0, 1, 1, 1, 0, 0, 0,
      0, 1, 1, 0, 3, 3, 3, 0, 0, 0, 1, 1, 1, 2, 0, 0,
      0, 0, 1, 0, 3, 0, 2, 2, 0, 0, 1, 1, 1, 1, 2, 0,
      2, 0, 1, 0, 3, 0, 2, 2, 2, 0, 1, 1, 1, 1, 1, 1,
      0, 0, 1, 0, 3, 2, 2, 2, 0, 0, 0, 0, 1, 1, 1, 1,
  };

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