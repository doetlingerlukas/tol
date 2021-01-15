#include <iostream>

#include <SFML/Graphics.hpp>

int main() {

  sf::RenderWindow window(sf::VideoMode(840, 600), "SFML works!", sf::Style::Titlebar | sf::Style::Close);

  // https://opengameart.org/node/8122
  sf::Texture floor_texture;
  floor_texture.setSmooth(true);
  if (!floor_texture.loadFromFile("textures/stonetiles.png")) {
    std::cerr << "Failed to load texture!" << std::endl;
  }

  sf::Sprite floor;
  floor.setTexture(floor_texture);

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
    window.draw(floor);
    window.display();
  }

  return 0;
}