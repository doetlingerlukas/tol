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

class Nuklear {
  std::shared_ptr<AssetCache> asset_cache;
  sf::Vector2u size;
  sf::Vector2f scale;
  sf::RenderWindow* window;
  std::reference_wrapper<const Stats> _stats;
  struct nk_context* ctx;

  struct nk_context* init(sf::RenderWindow* window) const {
    struct nk_context* ctx;
    ctx = nk_sfml_init(window);
    return ctx;
  }

  void push_window_state() const;
  void pop_window_state() const;

  [[nodiscard]] inline const Stats& stats() const {
    return _stats;
  }

  public:
  [[nodiscard]] struct nk_context* getCtx() const;

  void renderMenu(
    GameInstance& game, PlayState& play_state, const Character& player, const Inventory& inventory,
    const QuestStack& quests) const;

  void renderDeath(GameInstance& game, PlayState& play_state) const;

  void renderSettings(GameInstance& game, Settings& settings);

  void renderHud();

  std::pair<json, DialogState> renderResponseDialog(const json& dialog, DialogState dialog_state, const json& init);

  std::pair<json, DialogState> renderDialog(const json& lines, DialogState dialog_state);

  Nuklear(sf::Vector2u size_, const Stats& stats, std::shared_ptr<AssetCache> asset_cache_, sf::RenderWindow* _window);

  void setSize(sf::Vector2u size);

  void setScale(sf::Vector2f scale);
};

} // namespace tol
