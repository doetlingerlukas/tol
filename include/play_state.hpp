#pragma once

#include <SFML/Graphics.hpp>
#include <fmt/core.h>

#include <map.hpp>
#include <protagonist.hpp>
#include <input.hpp>
#include <game_state.hpp>

const float VIEW_MOVE_SPEED = 40.f;
const float VIEW_MOVE_ACCEL = 20.f;
const float VIEW_MOVE_DECEL = VIEW_MOVE_ACCEL * 2;
const float CHARACTER_MOVE_SPEED = 80.f;

class PlayState: public sf::Drawable {
  std::shared_ptr<AssetCache> asset_cache;

  sf::View map_view;
  std::reference_wrapper<TiledMap> map;
  std::reference_wrapper<Protagonist> player;

  sf::Vector2f scale;
  sf::Vector2f direction = { 0.0f, 0.0f };

  std::vector<sf::RectangleShape> collision_rects;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const;

  inline Protagonist& getPlayer() const {
    return player;
  }

  inline TiledMap& getMap() const {
    return map;
  }

public:
  PlayState(TiledMap& map_, Protagonist& player_, std::shared_ptr<AssetCache> asset_cache_, const sf::Vector2f& scale_, const sf::Vector2u& window_size);

  GameState update(KeyInput& key_input, const sf::RenderWindow& window, const std::chrono::milliseconds& now, float dt, std::optional<std::string>& npc_dialog);

  inline sf::Vector2f player_position() const {
    return getPlayer().getPosition();
  }

  inline void set_player_position(sf::Vector2f pos) {
    getPlayer().setPosition(pos);
  }
};
