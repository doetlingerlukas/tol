#include <iostream>
#include <algorithm>


#include <SFML/Graphics.hpp>

#include <map.hpp>

int main() {
  try {

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);
    window.setKeyRepeatEnabled(false);

    TiledMap map("map/map.json");

    bool up = false, down = false, left = false, right = false;

    float x_direction = 0;
    float y_direction = 0;

    while (window.isOpen()) {
      sf::Event event;

      while (window.pollEvent(event)) {
        switch (event.type) {
          case sf::Event::Closed:
            window.close();
            break;
          case sf::Event::KeyPressed:
          case sf::Event::KeyReleased: {
            switch (event.key.code) {
              case sf::Keyboard::Escape:
                window.close();
                break;
              case sf::Keyboard::Right:
                std::cout << "menuRight" << std::endl;
                break;
              case sf::Keyboard::D:
                right = event.type == sf::Event::KeyPressed;
                break;
              case sf::Keyboard::Left:
                std::cout << "menuLeft" << std::endl;
                break;
              case sf::Keyboard::A:
                left = event.type == sf::Event::KeyPressed;
                break;
              case sf::Keyboard::Up:
              std::cout << "menuUp" << std::endl;
                break;
              case sf::Keyboard::W:
                up = event.type == sf::Event::KeyPressed;
                break;
              case sf::Keyboard::Down:
                std::cout << "menuDown" << std::endl;
                break;
              case sf::Keyboard::S:
                down = event.type == sf::Event::KeyPressed;
                break;
              default:
                break;
            }

            break;
          }
          default:
            break;
        }
      }

      if (up && !down) {
        y_direction = std::clamp(y_direction + 1.0, 1.0, 10.0);
      } else if (down && !up) {
        y_direction = std::clamp(y_direction - 1.0, -10.0, -1.0);
      } else {
        if (y_direction > 0) {
          y_direction -= 1;
        } else if (y_direction < 0) {
          y_direction += 1;
        } else {
          y_direction = 0;
        }
      }

      if (right && !left) {
        x_direction = std::clamp(x_direction - 1.0, -10.0, -1.0);
      } else if (left && !right) {
        x_direction = std::clamp(x_direction + 1.0, 1.0, 10.0);
      } else {
        if (x_direction > 0) {
          x_direction -= 1;
        } else if (x_direction < 0) {
          x_direction += 1;
        } else {
          x_direction = 0;
        }
      }

      map.positionOffset.x += x_direction;
      map.positionOffset.y += y_direction;

      window.clear();
      window.draw(map);
      window.display();
    }

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
