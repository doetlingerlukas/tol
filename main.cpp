#include <sstream>
#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

const float VIEW_MOVE_SPEED = 60.f;
const float CHARACTER_MOVE_SPEED = 80.f;

int main() {
  try {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    sf::Font font;
    font.loadFromFile("assets/fonts/Gaegu-Regular.ttf");

    sf::View map_view;

    TiledMap map("assets/map.json");
    Character character("assets/tilesets/character-whitebeard.png");

    std::cout << map.getSize().x << ","  << map.getSize().y << std::endl;
    std::cout << map.getPosition().x << ", " << map.getPosition().y << std::endl;

    sf::Vector2f scale = { 2.0, 2.0 };
    map.setScale(scale);
    character.setScale(scale);

    std::cout << map.getPosition().x << ", " << map.getPosition().y << std::endl;

    sf::Vector2f direction = { 0.0f, 0.0f };
    bool up = false, down = false, left = false, right = false;
    bool w = false, a = false, s = false, d = false;


    bool menu_open = true;

    Menu menu;
    menu.add_item("PLAY", [&]() {
      window.setKeyRepeatEnabled(false);
      menu_open = false;
    });
    menu.add_item("LOAD GAME", [&]() { });
    menu.add_item("SAVE GAME", [&]() { });
    menu.add_item("EXIT", [&]() { window.close(); });

    sf::Clock clock;
    float dt = 0.0;

    map_view.reset({ 0, (map.getSize().y - window.getSize().y) * scale.y, (float)window.getSize().x, (float)window.getSize().y });

    while (window.isOpen()) {
      sf::Event event;

      dt = clock.restart().asSeconds();

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
                d = event.type == sf::Event::KeyPressed;
                break;
              case sf::Keyboard::Left:
                if (menu_open) {
                  left = false;
                } else {
                  left = event.type == sf::Event::KeyPressed;
                }
                break;
              case sf::Keyboard::A:
                a = event.type == sf::Event::KeyPressed;
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
                w = event.type == sf::Event::KeyPressed;
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
                s = event.type == sf::Event::KeyPressed;
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

      character.move(
        (a && !d) ? std::optional(LEFT) : ((d && !a) ? std::optional(RIGHT) : std::nullopt),
        (w && !s) ? std::optional(UP) : ((s && !w) ? std::optional(DOWN) : std::nullopt),
        dt * CHARACTER_MOVE_SPEED
      );

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

      map_view.setCenter(map_view.getCenter() + -direction * dt * VIEW_MOVE_SPEED);
      auto center = map_view.getCenter();

      window.clear();

      window.setView(map_view);
      window.draw(map);
      window.draw(character);

      auto coords = window.mapPixelToCoords({0, 0});

      window.setView(window.getDefaultView());

      if (menu_open) {
        window.draw(menu);
      }

      sf::Vector2u top_left_tile = {
        static_cast<unsigned>(std::max(0, (int)(coords.x / scale.x / map.getTileSize().x))),
        static_cast<unsigned>(std::max(0, (int)(coords.y / scale.y / map.getTileSize().y))),
      };

      std::stringstream ss;
      ss << "Map: " << map.getPosition().x << ", " << map.getPosition().y << "\n";
      ss << "Top Left Coords: " << coords.x << ", " << coords.y << "\n";
      ss << "Center Coords: " << center.x << ", " << center.y << "\n";
      ss << "Top Left Tile: " << top_left_tile.x << ", " << top_left_tile.y << "\n";
      ss << "Character: " << character.getPosition().x << ", " << character.getPosition().y << "\n";

      sf::Text text;
      text.setFont(font);
      text.setCharacterSize(16);
      text.setFillColor(sf::Color::White);
      text.setOutlineColor(sf::Color::Black);
      text.setOutlineThickness(1);
      text.setString(ss.str());
      text.setScale(scale);

      window.draw(text);

      window.display();
    }

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
