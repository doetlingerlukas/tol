#pragma once

#include <cstdarg>
#include <cstring>
#include <optional>

#include <nlohmann/json.hpp>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <SFML/Graphics.hpp>
#include <nuklear.h>
#include <nuklear_sfml_gl2.h>

#include "asset_cache.hpp"
#include "dialog_state.hpp"
#include "game_instance.hpp"
#include "game_state.hpp"
#include "settings.hpp"
#include "stats.hpp"

namespace tol {

using json = nlohmann::json;

class Game;

class Nuklear {
  std::shared_ptr<AssetCache> asset_cache;
  sf::Vector2u size;
  sf::Vector2f scale;
  sf::RenderWindow* window;
  std::reference_wrapper<const Stats> _stats;
  struct nk_context* _ctx;

  static struct nk_context* init(sf::RenderWindow* window);

  void push_window_state() const;
  void pop_window_state() const;

  [[nodiscard]] inline const Stats& stats() const {
    return _stats;
  }

  public:
  [[nodiscard]] inline struct nk_context* ctx() const {
    return _ctx;
  }

  void render_menu(Game& game, PlayState& play_state) const;

  void render_death(Game& game, PlayState& play_state) const;

  void render_settings(Game& game, Settings& settings);

  void render_hud();

  std::pair<json, DialogState> render_response_dialog(const json& dialog, DialogState dialog_state, const json& init);

  std::pair<json, DialogState> render_dialog(const json& lines, DialogState dialog_state);

  Nuklear(sf::Vector2u size_, const Stats& stats, std::shared_ptr<AssetCache> asset_cache_, sf::RenderWindow* _window);

  void setSize(sf::Vector2u size);

  void setScale(sf::Vector2f scale);
};

} // namespace tol

#ifndef TOL_GAME_HPP
#include "game.hpp"
#endif
