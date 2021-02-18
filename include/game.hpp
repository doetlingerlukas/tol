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

#include <dialog.hpp>
#include <fight.hpp>
#include <game_instance.hpp>
#include <game_state.hpp>
#include <input.hpp>
#include <inventory.hpp>
#include <map.hpp>
#include <music.hpp>
#include <overlay/info.hpp>
#include <overlay/overlay.hpp>
#include <play_state.hpp>
#include <protagonist.hpp>
#include <quest.hpp>
#include <settings.hpp>
#include <stats.hpp>
#include <string>

class Game {
  const std::string name = "Tales of Lostness";
  sf::RenderWindow window;
  fs::path dir;
  Settings& settings;

  GameInstance instance;

  sf::Vector2f scale;
  sf::Vector2f resolution_scale;
  sf::Uint32 window_style;

  std::shared_ptr<AssetCache> asset_cache;

  Info info;
  Protagonist player;

  bool mouse_pressed;

  void handle_event(
    sf::Event& event, KeyInput& key_input, tol::Music& music, Inventory& inventory, Overlay& overlay, Fight& fight);

  void handle_settings_update(tol::Music& music);

  public:
  Game(fs::path dir_, Settings& settings_);

  void run();
};
