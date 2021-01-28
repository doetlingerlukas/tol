#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

int main() {
  try {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    TiledMap map("assets/map.json");
    Character character("assets/tilesets/character-whitebeard.png");

    std::cout << map.getSize().x << ","  << map.getSize().y << std::endl;


    std::cout << map.positionOffset.x << ", " << map.positionOffset.y << std::endl;

    sf::Vector2f scale = { 2.0, 2.0 };
    map.setScale(scale);
    character.setScale(scale);

    map.positionOffset = { 0, window.getSize().y - map.getSize().y };
    std::cout << map.positionOffset.x << ", " << map.positionOffset.y << std::endl;

    sf::Vector2f direction = { 0.0f, 0.0f };
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
                if (menu_open) {
                  right = false;
                } else {
                  right = event.type == sf::Event::KeyPressed;
                }
                break;
              case sf::Keyboard::D:
                character.move(CharacterDirection::RIGHT);
                break;
              case sf::Keyboard::Left:
                if (menu_open) {
                  left = false;
                } else {
                  left = event.type == sf::Event::KeyPressed;
                }
                break;
              case sf::Keyboard::A:
                character.move(CharacterDirection::LEFT);
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
                character.move(CharacterDirection::UP);
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
                character.move(CharacterDirection::DOWN);
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
        direction.y = std::clamp(direction.y + 1.0, 1.0, 10.0);
      }
      else if (down && !up) {
        direction.y = std::clamp(direction.y - 1.0, -10.0, -1.0);
      }
      else {
        if (direction.y > 0) {
          direction.y -= 1;
        }
        else if (direction.y < 0) {
          direction.y += 1;
        }
        else {
          direction.y = 0;
        }
      }

      if (right && !left) {
        direction.x = std::clamp(direction.x - 1.0, -10.0, -1.0);
      }
      else if (left && !right) {
        direction.x = std::clamp(direction.x + 1.0, 1.0, 10.0);
      }
      else {
        if (direction.x > 0) {
          direction.x -= 1;
        }
        else if (direction.x < 0) {
          direction.x += 1;
        }
        else {
          direction.x = 0;
        }
      }

      map.set_position(map.positionOffset + direction, window);

      window.clear();
      window.draw(map);

      if (menu_open) {
        window.draw(menu);
      }

      window.draw(character);

      window.display();
    }

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
