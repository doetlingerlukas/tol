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

#include <map.hpp>
#include <string>
#include <character.hpp>
#include <settings.hpp>
#include <input.hpp>
#include <play_state.hpp>
#include <settings.hpp>
#include <stats.hpp>
#include <dialog.hpp>
#include <music.hpp>
#include <game_state.hpp>

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

  void handle_event(sf::Event& event, KeyInput& key_input, Menu& menu) {
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
      window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);
    }

    if (!settings.fullscreen() && window_style == sf::Style::Fullscreen) {
      window_style = window_style = sf::Style::Titlebar | sf::Style::Close;
      window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);
    }

    instance.setSettingsChanged(false);
  }

public:
  Game(Settings& settings_) : settings(settings_), instance(GameInstance()) {
    scale = { 2.0, 2.0 };
    resolution_scale = { 1.0, 1.0 };

    auto video_mode = sf::VideoMode::getDesktopMode();
    std::cout << "Full Resolution: " << video_mode.width << "," << video_mode.height << std::endl;

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
      window_style = sf::Style::Fullscreen;
    } else {
      window_style = sf::Style::Titlebar | sf::Style::Close;
    }
  }

  void run() {
    const auto [window_width, window_height] = settings.resolution();

    window.create(sf::VideoMode(window_width * resolution_scale.x, window_height * resolution_scale.y), name, window_style);
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
    tol::Music music(fs::path("assets/music"));
    music.play_background();

    sf::Clock clock;
    float dt = 0.0;
    std::chrono::milliseconds now = std::chrono::milliseconds(0);

    const std::shared_ptr<Nuklear> nuklear = std::make_shared<Nuklear>(window.getSize(), stats, asset_cache, &window);
    auto dialog = Dialog(nuklear);

    nuklear->setScale(scale);

    stats->health().subscribe([]() {
      std::exit(0);
    });

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
        window.pushGLStates();
        nuklear->renderMenu(instance);
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
        break;
      case GameState::PLAY:
      case GameState::QUEST:
      case GameState::FIGHT:
        play_state.update(key_input, window, now, dt);
        window.draw(play_state);

        window.pushGLStates();
        nuklear->renderHud();
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
        break;
      case GameState::DIALOG:
        window.draw(play_state);

        window.pushGLStates();
        instance.setState(dialog.show("npc1"));
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
        break;
      case GameState::SETTINGS:
        window.pushGLStates();
        nuklear->renderSettings(instance, settings);
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
