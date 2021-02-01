#include <sstream>
#include <iostream>
#include <algorithm>

#if __APPLE__
#include <CoreGraphics/CGDisplayConfiguration.h>

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else

#if _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <GL/gl.h>
#endif

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>
#define NK_IMPLEMENTATION
#define NK_SFML_GL2_IMPLEMENTATION
#include <nuklear.hpp>
#include <settings.hpp>

namespace fs = std::filesystem;

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

int main(int argc, char **argv) {
  try {
    const auto executeable_path = fs::canonical(argv[0]);
    const auto executeable_dir = executeable_path.parent_path();

    const auto settings = Settings(executeable_path);

    Game game(executeable_dir, settings);
    game.run();

    std::cout << "Scaled Resolution: " << width << "," << height << std::endl;

    resolution_scale.x = static_cast<float>(video_mode.width) / static_cast<float>(width);
    resolution_scale.y = static_cast<float>(video_mode.height) / static_cast<float>(height);
    #endif

    scale.x *= resolution_scale.x;
    scale.y *= resolution_scale.y;

    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;

    if (settings.fullscreen()) {
      style = sf::Style::Fullscreen;
    }

    sf::RenderWindow window(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), "Tales of Lostness", style);
    window.setVerticalSyncEnabled(settings.vsync());
    window.requestFocus();

    const std::shared_ptr<AssetCache> asset_cache = std::make_shared<AssetCache>("assets");

    sf::Font font;
    font.loadFromFile((asset_cache->dir() / "fonts/Gaegu-Regular.ttf").string());

    sf::View map_view;

    TiledMap map(asset_cache->dir() / "map.json", asset_cache);
    Character player(fs::path("tilesets/character-whitebeard.png"), asset_cache);
    map.setScale(scale);
    player.setScale(scale);

    map_view.reset({ 0, (map.getSize().y - window.getSize().y) * scale.y, (float)window.getSize().x, (float)window.getSize().y });
    map.addCharacter(&player);

    const auto spawn = map.getSpawn();
    if (spawn) {
      map_view.setCenter({ spawn->x * scale.x, spawn->y * scale.y });
      player.setPosition(*spawn);
    }
    const auto map_size = map.getSize();

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
    std::chrono::milliseconds now = std::chrono::milliseconds(0);

    const Nuklear& nuklear = Nuklear(window_width, window_height);
    auto ctx = nuklear.init(&window);

    while (window.isOpen()) {
      const auto millis = clock.restart().asMilliseconds();
      const auto dt = millis / 1000.f;
      now += std::chrono::milliseconds(millis);

      sf::Event event;
      nk_input_begin(ctx);
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

        nk_sfml_handle_event(&event);
      }
      nk_input_end(ctx);

      collision_rects = map.collisionTiles(player);

      player.move(
        (a && !d) ? std::optional(LEFT) : ((d && !a) ? std::optional(RIGHT) : std::nullopt),
        (w && !s) ? std::optional(UP) : ((s && !w) ? std::optional(DOWN) : std::nullopt),
        dt * CHARACTER_MOVE_SPEED, now, collision_rects, map_size
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

      map_view.setCenter(map.getView(window.getSize().x, window.getSize().y));
      auto center = map_view.getCenter();

      window.clear();

      std::stringstream ss;
      ss << "Center Coords: " << center.x << ", " << center.y << "\n";
      ss << "Player: " << player.getPosition().x << ", " << player.getPosition().y << "\n";
      ss << "Spawn: " << spawn->x << "," << spawn->y << "\n";

      map.update(map_view, window, now);

      window.setView(map_view);
      window.draw(map);

      for (auto shape : collision_rects) {
        shape.setScale(scale);
        auto position = shape.getPosition();
        shape.setPosition({ position.x * scale.x, position.y * scale.y });
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

      if (menu_open) {
        window.pushGLStates();
        nuklear.render_menu(ctx);
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
      } else {
        window.pushGLStates();
        nuklear.render_hud(ctx);
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
      }

      window.display();
    }

    nk_sfml_shutdown();
    return EXIT_SUCCESS;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
