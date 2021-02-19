#pragma once
#define TOL_PLAY_STATE_HPP

#include <optional>

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

#include <fmt/core.h>

#include "asset_cache.hpp"
#include "game_state.hpp"
#include "input.hpp"
#include "optional"
#include "overlay/info.hpp"
#include "quest.hpp"
#include "settings.hpp"

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

namespace tol {

using json = nlohmann::json;

class Map;
class Protagonist;

class PlayState: public sf::Drawable {
  std::shared_ptr<AssetCache> asset_cache;

  sf::View map_view;
  std::reference_wrapper<Map> _map;
  std::reference_wrapper<Protagonist> _player;
  std::reference_wrapper<QuestStack> _quest_stack;

  sf::Vector2f scale;
  sf::Vector2f direction = { 0.0f, 0.0f };

  std::vector<sf::RectangleShape> collision_shapes;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  public:
  [[nodiscard]] inline const Protagonist& player() const {
    return _player;
  }
  [[nodiscard]] inline Protagonist& player() {
    return _player;
  }

  [[nodiscard]] inline const Map& map() const {
    return _map;
  }
  [[nodiscard]] inline Map& map() {
    return _map;
  }

  [[nodiscard]] inline const QuestStack& quest_stack() const {
    return _quest_stack;
  }
  [[nodiscard]] inline QuestStack& quest_stack() {
    return _quest_stack;
  }

  PlayState(
    Map& _map, Protagonist& player, QuestStack& quest_stack, std::shared_ptr<AssetCache> asset_cache_,
    const sf::Vector2f& scale_, const sf::Vector2u& window_size);

  GameState update(
    KeyInput& key_input, const sf::RenderWindow& window, const std::chrono::milliseconds& now, float dt,
    std::optional<std::string>& npc_dialog, Info& info);

  void set_inventory(const json& inventory_array);

  void set_stats(const json& stats_array);

  void set_attacks(const json& attacks_array);

  [[nodiscard]] bool check_unlock_condition(const std::string& condition_name, bool collided);
};

} // namespace tol
#ifndef TOL_MAP_HPP
#include "map.hpp"
#endif

#ifndef TOL_PROTAGONIST_HPP
#include "protagonist.hpp"
#endif
