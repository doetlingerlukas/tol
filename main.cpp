#include <sstream>
#include <iostream>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>
#include <settings.hpp>

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

#if __APPLE__
#include <CoreGraphics/CGDisplayConfiguration.h>
#endif

int main(int argc, char **argv) {
  try {
    sf::Vector2f scale = { 2.0, 2.0 };
    sf::Vector2f resolution_scale = { 1.0, 1.0 };

    auto video_mode = sf::VideoMode::getDesktopMode();
    std::cout << "Full Resolution: " << video_mode.width << "," << video_mode.height << std::endl;

    const auto& settings = Settings(argv[0]);
    const auto[window_width, window_height] = settings.resolution();

    #if __APPLE__
    auto display_id = CGMainDisplayID();
    auto width = CGDisplayPixelsWide(display_id);
    auto height = CGDisplayPixelsHigh(display_id);

    std::cout << "Scaled Resolution: " << width << "," << height << std::endl;

    resolution_scale.x = static_cast<float>(video_mode.width) / static_cast<float>(width);
    resolution_scale.y = static_cast<float>(video_mode.height) / static_cast<float>(height);
    #endif

    scale.x *= resolution_scale.x;
    scale.y *= resolution_scale.y;

    sf::RenderWindow window(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    sf::Font font;
    font.loadFromFile("assets/fonts/Gaegu-Regular.ttf");

    sf::View map_view;

    TiledMap map("assets/map.json");
    Character player("assets/tilesets/character-whitebeard.png");
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
    menu.setScale(scale);

    std::vector<sf::RectangleShape> collision_rects;

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

      collision_rects = map.collisionTiles(player);

      player.move(
        (a && !d) ? std::optional(LEFT) : ((d && !a) ? std::optional(RIGHT) : std::nullopt),
        (w && !s) ? std::optional(UP) : ((s && !w) ? std::optional(DOWN) : std::nullopt),
        dt * CHARACTER_MOVE_SPEED, collision_rects
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

      std::stringstream ss;
      ss << "Center Coords: " << center.x << ", " << center.y << "\n";
      ss << "Player: " << player.getPosition().x << ", " << player.getPosition().y << "\n";
      ss << "Spawn: " << spawn->x << "," << spawn->y << "\n";

      map.update(map_view, window);

      window.setView(map_view);
      window.draw(map);

      for (auto shape : collision_rects) {
        window.draw(shape);
      }

      window.setView(window.getDefaultView());

      if (menu_open) {
        window.draw(menu);
      }

      sf::Text text;
      text.setFont(font);
      text.setCharacterSize(16 * scale.y);
      text.setFillColor(sf::Color::White);
      text.setOutlineColor(sf::Color::Black);
      text.setOutlineThickness(1);
      text.setString(ss.str());

      window.draw(text);

      window.display();
    }

    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
