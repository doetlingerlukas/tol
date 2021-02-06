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
#include <menu.hpp>
#include <character.hpp>
#include <settings.hpp>
#include <input.hpp>
#include <play_state.hpp>
#include <settings.hpp>
#include <stats.hpp>
#include <dialog.hpp>
#include <music.hpp>
#include <game-state.hpp>


class Game {
  sf::RenderWindow window;
  Settings& settings;
  fs::path dir;

  GameInstance instance;

  sf::Vector2f scale;

  void handle_event(sf::Event& event, KeyInput& key_input) {
    const auto state = instance.getState();

    switch (event.type) {
    case sf::Event::Closed:
      window.close();
      break;

    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased: {
      switch (event.key.code) {

      case sf::Keyboard::Q:
        if (state == GameState::PLAY)
          instance.setState(GameState::DIALOG);
        else
          instance.setState(GameState::PLAY);
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

  void handle_settings_update() {
    std::cout << "V-Sync: " << settings.vsync() << std::endl;
    std::cout << "Fullscreen: " << settings.fullscreen() << std::endl;
    std::cout << "Volume: " << settings.volume_level << std::endl;

    instance.setSettingsChanged(false);
  }

public:
  Game(Settings& settings_) : settings(settings_), instance(GameInstance()) {}

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
    tol::Music music(fs::path("assets/music"));
    music.play_background();

    sf::Clock clock;
    float dt = 0.0;
    std::chrono::milliseconds now = std::chrono::milliseconds(0);

    const std::shared_ptr<Nuklear> nuklear = std::make_shared<Nuklear>(window.getSize(), stats, asset_cache, &window);
    const auto dialog = Dialog(nuklear);

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
        handle_event(event, key_input, menu, music);
      }
      nk_input_end(nuklear->getCtx());

      if (instance.isSettingsChanged()) {
        handle_settings_update();
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
        dialog.show("npc1");
        //nuklear->renderDialog();
        nk_sfml_render(NK_ANTI_ALIASING_ON);
        window.popGLStates();
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
