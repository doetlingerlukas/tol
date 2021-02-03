#pragma once

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>
#include <settings.hpp>
#include <input.hpp>

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

#if __APPLE__
#include <CoreGraphics/CGDisplayConfiguration.h>
#endif

enum GameState {
  MENU = 0,
  PLAY
};

class Game {
  sf::RenderWindow window;
  const Settings& settings;

  GameState state;

  sf::Vector2f scale;

  void handle_event(sf::Event& event, KeyInput& key_input, Menu& menu) {
    switch (event.type) {
    case sf::Event::Closed:
      window.close();
      break;

    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased: {
      switch (event.key.code) {
      case sf::Keyboard::Escape:
        state = MENU;
        window.setKeyRepeatEnabled(true);
        break;
      case sf::Keyboard::Right:
        if (state == MENU) {
          key_input.right = false;
        }
        else {
          key_input.right = event.type == sf::Event::KeyPressed;
        }
        break;
      case sf::Keyboard::D:
        key_input.d = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Left:
        if (state == MENU) {
          key_input.left = false;
        }
        else {
          key_input.left = event.type == sf::Event::KeyPressed;
        }
        break;
      case sf::Keyboard::A:
        key_input.a = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Up:
        if (state == MENU) {
          if (event.type == sf::Event::KeyPressed) {
            menu.up();
          }
          key_input.up = false;
        }
        else {
          key_input.up = event.type == sf::Event::KeyPressed;
        }
        break;
      case sf::Keyboard::W:
        key_input.w = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Down:
        if (state == MENU) {
          if (event.type == sf::Event::KeyPressed) {
            menu.down();
          }
          key_input.down = false;
        }
        else {
          key_input.down = event.type == sf::Event::KeyPressed;
        }
        break;
      case sf::Keyboard::S:
        key_input.s = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Enter:
        if (state == MENU) {
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

public:
  Game(const Settings& settings_) : settings(settings_), state(MENU) {}

  void run() {
    scale = { 2.0, 2.0 };
    sf::Vector2f resolution_scale = { 1.0, 1.0 };

    auto video_mode = sf::VideoMode::getDesktopMode();
    std::cout << "Full Resolution: " << video_mode.width << "," << video_mode.height << std::endl;

    const auto [window_width, window_height] = settings.resolution();

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

    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close;

    if (settings.fullscreen()) {
      style = sf::Style::Fullscreen;
    }

    window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), "Tales of Lostness", style);
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
    KeyInput key_input;

    Menu menu;
    menu.add_item("PLAY", [this]() {
      window.setKeyRepeatEnabled(false);
      state = PLAY;
      });
    menu.add_item("LOAD GAME", [&]() {});
    menu.add_item("SAVE GAME", [&]() {});
    menu.add_item("EXIT", [&]() { window.close(); });
    menu.setScale(scale);

    std::vector<sf::RectangleShape> collision_rects;

    sf::Clock clock;
    float dt = 0.0;
    std::chrono::milliseconds now = std::chrono::milliseconds(0);

    while (window.isOpen()) {
      const auto millis = clock.restart().asMilliseconds();
      const auto dt = millis / 1000.f;
      now += std::chrono::milliseconds(millis);

      sf::Event event;
      while (window.pollEvent(event)) {
        handle_event(event, key_input, menu);
      }

      if (state == PLAY) {
        collision_rects = map.collisionTiles(player);

        player.move(
          (key_input.a && !key_input.d) ? std::optional(LEFT) : ((key_input.d && !key_input.a) ? std::optional(RIGHT) : std::nullopt),
          (key_input.w && !key_input.s) ? std::optional(UP) : ((key_input.s && !key_input.w) ? std::optional(DOWN) : std::nullopt),
          dt * CHARACTER_MOVE_SPEED, now, collision_rects, map_size
        );


        if (key_input.up && !key_input.down) {
          direction.y = std::clamp(direction.y + 1.0 * dt * VIEW_MOVE_ACCEL, 1.0, 25.0);
        }
        else if (key_input.down && !key_input.up) {
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

        if (key_input.right && !key_input.left) {
          direction.x = std::clamp(direction.x - 1.0 * dt * VIEW_MOVE_ACCEL, -25.0, -1.0);
        }
        else if (key_input.left && !key_input.right) {
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

      if (state == MENU) {
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
  }
};