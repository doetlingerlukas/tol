#include <sstream>
#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
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
    Character player("assets/tilesets/character-whitebeard.png");

    sf::Vector2f scale = { 2.0, 2.0 };
    map.setScale(scale);
    player.setScale(scale);

    map_view.reset({ 0, (map.getSize().y - window.getSize().y) * scale.y, (float)window.getSize().x, (float)window.getSize().y });
    map.addCharacter(&player);

    const auto spawn = map.getSpawn();
    if (spawn) {
      map_view.setCenter({ spawn->x * scale.x, spawn->y * scale.y });
      player.setPosition(*spawn);
    }

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

      player.move(
        (a && !d) ? std::optional(LEFT) : ((d && !a) ? std::optional(RIGHT) : std::nullopt),
        (w && !s) ? std::optional(UP) : ((s && !w) ? std::optional(DOWN) : std::nullopt),
        dt * CHARACTER_MOVE_SPEED
      );

      if (up && !down) {
        direction.y = std::clamp(direction.y + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
      }
      else if (down && !up) {
        direction.y = std::clamp(direction.y - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
      }
      else {
        if (direction.y >= 0.5) {
          direction.y -= 1.0 * dt * VIEW_MOVE_DECEL;
        }
        else if (direction.y <= -0.5) {
          direction.y += 1.0 * dt * VIEW_MOVE_DECEL;
        }
        else {
          direction.y = 0;
        }
      }

      if (right && !left) {
        direction.x = std::clamp(direction.x - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
      }
      else if (left && !right) {
        direction.x = std::clamp(direction.x + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
      }
      else {
        if (direction.x >= 0.5) {
          direction.x -= 1.0 * dt * VIEW_MOVE_DECEL;
        }
        else if (direction.x <= -0.5) {
          direction.x += 1.0 * dt * VIEW_MOVE_DECEL;
        }
        else {
          direction.x = 0;
        }
      }

      map_view.setCenter(map_view.getCenter() + -direction * dt * VIEW_MOVE_SPEED);
      auto center = map_view.getCenter();

      window.clear();

      window.setView(map_view);

      auto coords = window.mapPixelToCoords({0, 0});

      sf::Vector2u top_left_tile = {
        static_cast<unsigned>(std::max(0, (int)(coords.x / scale.x / map.getTileSize().x))),
        static_cast<unsigned>(std::max(0, (int)(coords.y / scale.y / map.getTileSize().y))),
      };

      size_t from_x = std::max(0, (int)coords.x);
      size_t to_x = std::max(0, (int)coords.x + (int)window.getSize().x);
      size_t from_y = std::max(0, (int)coords.y);
      size_t to_y = std::max(0, (int)coords.y + (int)window.getSize().y);

      map.update(
        from_x, to_x,
        from_y, to_y
      );

      std::stringstream ss;
      ss << "Top Left Coords: " << coords.x << ", " << coords.y << "\n";
      ss << "Center Coords: " << center.x << ", " << center.y << "\n";
      ss << "Top Left Tile: " << top_left_tile.x << ", " << top_left_tile.y << "\n";
      ss << "Player: " << player.getPosition().x << ", " << player.getPosition().y << "\n";
      ss << "Visible Map: " << from_x << "," << from_y << " -> " << to_x << "," << to_y << "\n";
      ss << "Spawn: " << spawn->x << "," << spawn->y << "\n";

      window.draw(map);

      window.setView(window.getDefaultView());

      if (menu_open) {
        window.draw(menu);
      }

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
