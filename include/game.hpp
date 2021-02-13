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

#include <nuklear.hpp>

#include <SFML/Graphics.hpp>
#include <fmt/core.h>

#include <map.hpp>
#include <string>
#include <protagonist.hpp>
#include <settings.hpp>
#include <input.hpp>
#include <play_state.hpp>
#include <settings.hpp>
#include <stats.hpp>
#include <dialog.hpp>
#include <music.hpp>
#include <game_state.hpp>
#include <game_instance.hpp>
#include <overlay/info.hpp>

class Game {
  const std::string name = "Tales of Lostness";
  sf::RenderWindow window;
  Settings& settings;
  fs::path dir;

  GameInstance instance;

  sf::Vector2f scale;
  sf::Vector2f resolution_scale;
  sf::Uint32 window_style;

  bool mouse_pressed;

  void handle_event(sf::Event& event, KeyInput& key_input, tol::Music& music) {
    const auto state = instance.getState();

    switch (event.type) {
      case sf::Event::Closed:
        window.close();
        break;
      case sf::Event::MouseMoved:
        if (state == GameState::MENU) {
          //menu.mouse({ event.mouseMove.x, event.mouseMove.y }, mouse_pressed);
        }
        break;
      case sf::Event::MouseButtonPressed:
      case sf::Event::MouseButtonReleased:
        if (state == GameState::MENU) {
          mouse_pressed = event.type == sf::Event::MouseButtonPressed;

          if (event.mouseButton.button == sf::Mouse::Button::Left) {
            //menu.mouse({ event.mouseButton.x, event.mouseButton.y }, mouse_pressed);
          }
        } else {
          mouse_pressed = false;
        }
      break;

    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased: {
      switch (event.key.code) {
        case sf::Keyboard::Q:
        instance.setState(GameState::DIALOG);
        break;
      case sf::Keyboard::Escape:
        instance.setState(GameState::MENU);
        window.setKeyRepeatEnabled(true);
        break;
      case sf::Keyboard::Right:
        key_input.right = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::D:
        key_input.d = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Left:
        key_input.left = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::A:
        key_input.a = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Up:
        key_input.up = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::W:
        key_input.w = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::Down:
        key_input.down = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::S:
        key_input.s = event.type == sf::Event::KeyPressed;
        break;
      case sf::Keyboard::E:
        key_input.e = event.type == sf::Event::KeyPressed;
      case sf::Keyboard::Enter:
        break;
      case sf::Keyboard::M:
        music.stop_background();
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

  void handle_settings_update(tol::Music& music) {
    window.setVerticalSyncEnabled(settings.vsync());
    music.set_volume(settings.volume_level);

    const auto [window_width, window_height] = settings.resolution();

    if (settings.fullscreen() && window_style != sf::Style::Fullscreen) {
      window_style = sf::Style::Fullscreen;
    }

    if (!settings.fullscreen() && window_style == sf::Style::Fullscreen) {
      window_style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
    }

    window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);

    instance.setSettingsChanged(false);
  }

public:
  Game(fs::path dir_, Settings& settings_) : dir(dir_), settings(settings_), instance(GameInstance(dir_)) {
    scale = { 2.0, 2.0 };
    resolution_scale = { 1.0, 1.0 };

    auto video_mode = sf::VideoMode::getDesktopMode();
    fmt::print("Full Resolution: {}, {}\n", video_mode.width, video_mode.height);

    #if __APPLE__
    auto display_id = CGMainDisplayID();
    auto width = CGDisplayPixelsWide(display_id);
    auto height = CGDisplayPixelsHigh(display_id);
    fmt::print("Scaled Resolution: {}, {}\n", width, height);

    resolution_scale.x = static_cast<float>(video_mode.width) / static_cast<float>(width);
    resolution_scale.y = static_cast<float>(video_mode.height) / static_cast<float>(height);
    #endif

    scale.x *= resolution_scale.x;
    scale.y *= resolution_scale.y;

    if (settings.fullscreen()) {
      window_style = sf::Style::Fullscreen;
    } else {
      window_style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
    }
  }

  void run() {
    const auto [window_width, window_height] = settings.resolution();

    window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);
    window.setVerticalSyncEnabled(settings.vsync());
    window.requestFocus();

    const std::shared_ptr<AssetCache> asset_cache = std::make_shared<AssetCache>(dir / "assets");

    json protagonist_stats = {
      { "strength", 10 },
      { "speed",  10 },
      { "health", 100 },
      { "level",  1 }
    };

    const std::shared_ptr<Stats> stats = std::make_shared<Stats>(std::move(protagonist_stats));

    TiledMap map(asset_cache->dir() / "map.json", asset_cache);
    Protagonist player(fs::path("tilesets/character-whitebeard.png"), asset_cache, stats, "detlef");

    map.setScale(scale);
    player.setScale(scale);

    map.setPlayer(&player);

    PlayState play_state(map, player, asset_cache, scale, window.getSize());
    KeyInput key_input;
    tol::Music music(fs::path("assets/music"), settings.volume_level);
    music.play_background();

    Info info(asset_cache);
    auto loost_generator = [](int n) {
      std::ostringstream os;
      for (int i = 0; i < n; i++)
        os << "loost ";
      return os.str();
    };
    info.display_info(loost_generator(10), std::chrono::seconds(10));

    sf::Clock clock;
    float dt = 0.0;
    std::chrono::milliseconds now = std::chrono::milliseconds(0);

    const std::shared_ptr<Nuklear> nuklear = std::make_shared<Nuklear>(window.getSize(), stats, asset_cache, &window);
    auto dialog = Dialog(nuklear);

    nuklear->setScale(scale);

    stats->health().subscribe([]() {
      std::exit(0);
    });

    std::optional<std::string> last_npc_dialog;

    while (window.isOpen()) {
      const auto millis = clock.restart().asMilliseconds();
      const auto dt = millis / 1000.f;
      now += std::chrono::milliseconds(millis);

      sf::Event event;
      nk_input_begin(nuklear->getCtx());
      while (window.pollEvent(event)) {
        handle_event(event, key_input, music);
      }
      nk_input_end(nuklear->getCtx());

      if (instance.isSettingsChanged()) {
        handle_settings_update(music);
        nuklear->setSize(window.getSize());
      }

      window.clear();
      window.resetGLStates();

      switch (instance.getState()) {
      case GameState::QUIT:
        window.close();
        break;
      case GameState::MENU:
        nuklear->renderMenu(instance, play_state);
        break;
      case GameState::PLAY:
      case GameState::QUEST:
      case GameState::FIGHT:
        instance.setState(play_state.update(key_input, window, now, dt, last_npc_dialog));
        window.draw(play_state);

        info.update_time(std::chrono::milliseconds(millis));
        window.draw(info);

        nuklear->renderHud();
        break;
      case GameState::DIALOG:
        window.draw(play_state);

        if (last_npc_dialog)
          instance.setState(dialog.show("npc1"));
        break;
      case GameState::SETTINGS:
        nuklear->renderSettings(instance, settings);
        break;
      default:
        break;
      }

      window.display();
    }

    nk_sfml_shutdown();
  }
};
