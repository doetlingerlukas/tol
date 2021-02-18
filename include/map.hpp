#pragma once
#define TOL_MAP_HPP

#include <algorithm>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <variant>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

#include <animation.hpp>
#include <asset_cache.hpp>
#include <character.hpp>
#include <npc.hpp>
#include <object.hpp>
#include <overlay/info.hpp>
#include <protagonist.hpp>
#include <tile.hpp>

class PlayState;

struct Collision {
  sf::FloatRect bounds;
  std::optional<std::reference_wrapper<const std::string>> unlock_condition;
  std::optional<std::reference_wrapper<const std::string>> unlock_hint;
};

class TiledMap: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  std::unique_ptr<tson::Map> map;
  std::map<int, Object> collectibles;

  size_t from_x;
  size_t to_x;
  size_t from_y;
  size_t to_y;

  Protagonist* player;
  std::vector<Npc> npcs;
  std::vector<const Character*> characters;
  std::chrono::milliseconds now;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
  void drawLayer(
    tson::Layer& layer, sf::RenderTarget& target,
    std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const;

  mutable std::map<int, Animation> running_animations;

  void drawTileLayer(
    tson::Layer& layer, sf::RenderTarget& target,
    std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const;

  void drawImageLayer(tson::Layer& layer, sf::RenderTarget& target) const;

  void drawObjectLayer(
    tson::Layer& layer, sf::RenderTarget& target,
    std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const;

  void createTileData(tson::Layer& layer);
  void gatherCollectibles(tson::Layer& layer);

  sf::FloatRect window_rect;

  public:
  sf::Vector2i getTileSize() const;

  sf::Vector2f getSize() const;

  TiledMap(const fs::path& map_path, std::shared_ptr<AssetCache> asset_cache_);

  sf::Vector2i mapCoordsToTile(const sf::Vector2f& coords);

  inline sf::FloatRect getWindowRect() {
    return window_rect;
  }

  void update(const sf::View& view, const sf::RenderWindow& window, const std::chrono::milliseconds& now);

  void setPosition(sf::Vector2f position, const sf::RenderTarget& target);

  std::optional<sf::Vector2f> getSpawn();

  sf::Vector2f getView(float window_width, float window_height);

  void setPlayer(Protagonist* player);

  void addCharacter(const Character* character);

  std::map<int, Object>& getCollectibles();

  std::vector<Collision> collisions_around(const sf::FloatRect& player) const;

  void setScale(float factorX, float factorY);

  void setScale(sf::Vector2f factors);

  std::vector<const Character*> getCharacters() const;

  std::vector<Npc>& getNpcs();

  void setPlayer(Character* player);

  std::vector<sf::RectangleShape> collisionTiles(const Character& player) const;
};

#ifndef TOL_PLAY_STATE_HPP
#include <play_state.hpp>
#endif
