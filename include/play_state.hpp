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


const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

namespace tol {

using json = nlohmann::json;

class TiledMap;
class Protagonist;

class PlayState: public sf::Drawable {
  std::shared_ptr<AssetCache> asset_cache;

  sf::View map_view;
  std::reference_wrapper<TiledMap> map;
  std::reference_wrapper<Protagonist> player;
  std::reference_wrapper<QuestStack> quest_stack;

  sf::Vector2f scale;
  sf::Vector2f direction = { 0.0f, 0.0f };

  std::vector<sf::RectangleShape> collision_shapes;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  public:
  [[nodiscard]] inline Protagonist& getPlayer() const {
    return player;
  }

  [[nodiscard]] inline TiledMap& getMap() const {
    return map;
  }

  [[nodiscard]] inline QuestStack& getQuestStack() const {
    return quest_stack;
  }

  PlayState(
    TiledMap& map_, Protagonist& player_, QuestStack& quest_stack_, std::shared_ptr<AssetCache> asset_cache_,
    const sf::Vector2f& scale_, const sf::Vector2u& window_size);

  GameState update(
    KeyInput& key_input, const sf::RenderWindow& window, const std::chrono::milliseconds& now, float dt,
    std::optional<std::string>& npc_dialog, Info& info);

  [[nodiscard]] sf::Vector2f player_position() const;

  void set_player_position(sf::Vector2f pos);
  void set_inventory(const json& inventory_array);

  [[nodiscard]] bool check_unlock_condition(const std::string& condition_name, bool collided) const;
};

} // namespace tol
#ifndef TOL_MAP_HPP
#include <map.hpp>
#endif

#ifndef TOL_PROTAGONIST_HPP
#include <protagonist.hpp>
#endif
