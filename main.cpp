#include <iostream>

#include <SFML/Graphics.hpp>

#include <map.hpp>

int main() {
  try {

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);

    TiledMap map("map/map.json");

    while (window.isOpen()) {
      sf::Event event;

      while (window.pollEvent(event)) {
        switch (event.type) {
          case sf::Event::Closed:
            window.close();
            break;
          case sf::Event::KeyPressed:
            switch (event.key.code) {
              case sf::Keyboard::Escape:
                window.close();
                break;
              case sf::Keyboard::Right:
                std::cout << "menuRight" << std::endl;
                break;
              case sf::Keyboard::D:
                map.positionOffset.x -= 32.0;
                break;
              case sf::Keyboard::Left:
                std::cout << "menuLeft" << std::endl;
                break;
              case sf::Keyboard::A:
                map.positionOffset.x += 32.0;
                break;
              case sf::Keyboard::Up:
                std::cout << "menuUp" << std::endl;
                break;
              case sf::Keyboard::W:
                map.positionOffset.y += 32.0;
                break;
              case sf::Keyboard::Down:
                std::cout << "menuDown" << std::endl;
                break;
              case sf::Keyboard::S:
                map.positionOffset.y -= 32.0;
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }
      }

      window.clear(sf::Color::Black);
      window.draw(map);
      window.display();
    }

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
