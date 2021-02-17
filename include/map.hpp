#pragma once
#define _MAP_HPP_

#include <variant>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

#include <asset_cache.hpp>
#include <tile.hpp>
#include <object.hpp>
#include <animation.hpp>
#include <character.hpp>
#include <npc.hpp>
#include <protagonist.hpp>
#include <overlay/info.hpp>

class PlayState;

struct Collision {
  sf::FloatRect bounds;
  std::optional<std::reference_wrapper<const std::string>> unlock_condition;
  std::optional<std::reference_wrapper<const std::string>> unlock_hint;
};

class TiledMap: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  fs::path dir;
  fs::path filename;

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

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const;
  void drawLayer(tson::Layer& layer, sf::RenderTarget& target, std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const;

  mutable std::map<int, Animation> running_animations;

  void drawTileLayer(tson::Layer& layer, sf::RenderTarget& target, std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const;

  void drawImageLayer(tson::Layer& layer, sf::RenderTarget& target) const;

  void drawObjectLayer(tson::Layer& layer, sf::RenderTarget& target, std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const;

  void createTileData(tson::Layer& layer);
  void gatherCollectibles(tson::Layer& layer);

public:
  sf::Vector2i getTileSize() const;

  sf::Vector2f getSize() const;

  TiledMap(const fs::path& map_path, const std::shared_ptr<AssetCache> asset_cache_);

  sf::Vector2i mapCoordsToTile(const sf::Vector2f& coords);

  inline sf::FloatRect getWindowRect() {
    return window_rect;
  }

  sf::FloatRect window_rect;

  void update(const sf::View& view, const sf::RenderWindow& window, const std::chrono::milliseconds& now);

  void setPosition(sf::Vector2f position, const sf::RenderTarget& target);

  std::optional<sf::Vector2f> getSpawn();

  sf::Vector2f getView(const float window_width, const float window_height);

  void setPlayer(Protagonist* player);

  void addCharacter(const Character* character);

  std::map<int, Object>& getCollectibles();

  std::vector<Collision> collisions_around(const sf::FloatRect& player) const;

  void setScale(float factorX, float factorY);

  void setScale(const sf::Vector2f factors);

  std::vector<const Character*> getCharacters() const;

  std::vector<Npc>& getNpcs();

  void setPlayer(Character* player);

  std::vector<sf::RectangleShape> collisionTiles(const Character& player) const;
};

#ifndef _PLAY_STATE_HPP_
#include <play_state.hpp>
#endif
