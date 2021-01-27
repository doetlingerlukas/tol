#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

int main() {
  try {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    TiledMap map("assets/map.json");

    sf::Vector2f position = { 0.0f, 0.0f };
    bool up = false, down = false, left = false, right = false;

    bool menu_open = true;

    Menu menu;
    menu.add_item("PLAY", [&]() {
      window.setKeyRepeatEnabled(false);
      menu_open = false;
    });
    menu.add_item("LOAD GAME", [&]() { });
    menu.add_item("SAVE GAME", [&]() { });
    menu.add_item("EXIT", [&]() { window.close(); });

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
                menu_open = true;
                window.setKeyRepeatEnabled(true);
                break;
              case sf::Keyboard::Right:
                right = event.type == sf::Event::KeyPressed;
                break;
              case sf::Keyboard::D:
                break;
              case sf::Keyboard::Left:
                left = event.type == sf::Event::KeyPressed;
                break;
              case sf::Keyboard::A:
                break;
              case sf::Keyboard::Up:
                if (menu_open) {
                  if (event.type == sf::Event::KeyPressed) {
                    menu.up();
                  }
                  up = false;
                } else {
                  up = event.type == sf::Event::KeyPressed;
                }
                break;
              case sf::Keyboard::W:
                break;
              case sf::Keyboard::Down:
                if (menu_open) {
                  if (event.type == sf::Event::KeyPressed) {
                    menu.down();
                  }
                  down = false;
                } else {
                  down = event.type == sf::Event::KeyPressed;
                }
                break;
              case sf::Keyboard::S:
                break;
              case sf::Keyboard::Enter:
                if (menu_open) {
                  menu.enter(event.type == sf::Event::KeyPressed);
                }
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
        position.y = std::clamp(position.y + 1.0, 1.0, 10.0);
      }
      else if (down && !up) {
        position.y = std::clamp(position.y - 1.0, -10.0, -1.0);
      }
      else {
        if (position.y > 0) {
          position.y -= 1;
        }
        else if (position.y < 0) {
          position.y += 1;
        }
        else {
          position.y = 0;
        }
      }

      if (right && !left) {
        position.x = std::clamp(position.x - 1.0, -10.0, -1.0);
      }
      else if (left && !right) {
        position.x = std::clamp(position.x + 1.0, 1.0, 10.0);
      }
      else {
        if (position.x > 0) {
          position.x -= 1;
        }
        else if (position.x < 0) {
          position.x += 1;
        }
        else {
          position.x = 0;
        }
      }

      map.positionOffset += position;

      window.clear();
      window.draw(map);

      if (menu_open) {
        window.draw(menu);
      }

      window.display();
    }

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
