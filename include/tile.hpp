#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <tileson.hpp>

#include "animation.hpp"
#include "asset_cache.hpp"
#include "z_indexable.hpp"

namespace tol {

class Tile: public ZIndexable, public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  static std::map<int, Animation> running_animations;

  std::reference_wrapper<tson::Tile> _tile;

  std::optional<float> _z_index;

  std::chrono::milliseconds now;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  protected:
  inline tson::Tile& tile() const {
    return _tile;
  }

  public:
  Tile(tson::Tile& tile, const sf::Vector2f& position, std::shared_ptr<AssetCache> asset_cache_);

  Tile(const tson::TileObject& object, std::shared_ptr<AssetCache> asset_cache);

  virtual std::optional<float> z_index() const override;

  sf::FloatRect bounds() const;

  std::vector<sf::FloatRect> collisions() const;

  void update(std::chrono::milliseconds now);
};

} // namespace tol
