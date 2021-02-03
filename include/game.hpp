#pragma once

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

#define NK_IMPLEMENTATION
#define NK_SFML_GL2_IMPLEMENTATION
#include <nuklear.hpp>

#include <SFML/Graphics.hpp>

#include <map.hpp>
#include <menu.hpp>
#include <character.hpp>
#include <settings.hpp>
#include <input.hpp>
#include <play_state.hpp>
#include <settings.hpp>
#include <stats.hpp>


enum class GameState {
  MENU,
  PLAY
};

class Game {
  sf::RenderWindow window;
  const Settings& settings;
  fs::path dir;

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
        state = GameState::MENU;
        window.setKeyRepeatEnabled(true);
        break;
      case sf::Keyboard::Right:
        if (state == GameState::MENU) {
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
        if (state == GameState::MENU) {
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
        if (state == GameState::MENU) {
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
        if (state == GameState::MENU) {
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
        if (state == GameState::MENU) {
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

public:
  Game(const fs::path dir, const Settings& settings_): dir(dir), settings(settings_), state(GameState::MENU) {}

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

    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;

    if (settings.fullscreen()) {
      style = sf::Style::Fullscreen;
    }

    window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), "Tales of Lostness", style);
    window.setVerticalSyncEnabled(settings.vsync());
    window.requestFocus();

    const std::shared_ptr<AssetCache> asset_cache = std::make_shared<AssetCache>(dir / "assets");

    const std::shared_ptr<Stats> stats = std::make_shared<Stats>();

    TiledMap map(asset_cache->dir() / "map.json", asset_cache);
    Character player(fs::path("tilesets/character-whitebeard.png"), asset_cache, stats);

    map.setScale(scale);
    player.setScale(scale);

    map.addCharacter(&player);

    PlayState play_state(&map, &player, asset_cache, scale, window.getSize());

    KeyInput key_input;

    Menu menu;
    menu.add_item("PLAY", [this]() {
      window.setKeyRepeatEnabled(false);
      state = GameState::PLAY;
      });
    menu.add_item("LOAD GAME", [&]() {});
    menu.add_item("SAVE GAME", [&]() {});
    menu.add_item("EXIT", [&]() { 
      window.close();
      std::exit(0);
    });
    menu.setScale(scale);

    sf::Clock clock;
    float dt = 0.0;
    std::chrono::milliseconds now = std::chrono::milliseconds(0);

    Nuklear nuklear = Nuklear(window_width, window_height, stats);
    auto ctx = nuklear.init(&window);

    while (window.isOpen()) {
      const auto millis = clock.restart().asMilliseconds();
      const auto dt = millis / 1000.f;
      now += std::chrono::milliseconds(millis);

      sf::Event event;
      nk_input_begin(ctx);
      while (window.pollEvent(event)) {
        handle_event(event, key_input, menu);
      }
      nk_input_end(ctx);

      window.clear();

      switch (state) {
      case GameState::MENU:
        window.draw(menu);

        window.pushGLStates();
        nuklear.render_menu(ctx);
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
        break;
      case GameState::PLAY:
        play_state.update(key_input, window, now, dt);
        window.draw(play_state);

        window.pushGLStates();
        nuklear.render_hud(ctx);
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
        break;
      default:
        break;
      }

      window.display();
    }

    nk_sfml_shutdown();
  }
};
