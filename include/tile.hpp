#pragma once

#include <SFML/Graphics.hpp>

#include <nlohmann/json.hpp>
#include <tileson.hpp>

#include <animation.hpp>
#include <asset_cache.hpp>
#include <z_indexable.hpp>

class Tile: public ZIndexable, public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  static std::map<int, Animation> running_animations;

  std::reference_wrapper<tson::Tile> tile;

  std::optional<float> z_index;

  std::chrono::milliseconds now;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const;

  protected:
  inline tson::Tile& getTile() const {
    return tile;
  }

  public:
  Tile(tson::Tile& tile_, const sf::Vector2f& position, std::shared_ptr<AssetCache> asset_cache_);

  Tile(const tson::TileObject& object, std::shared_ptr<AssetCache> asset_cache);

  virtual std::optional<float> zIndex() const;

  sf::FloatRect getBoundingRect() const;

  std::vector<sf::FloatRect> getCollisionRects() const;

  void update(std::chrono::milliseconds now);
};
