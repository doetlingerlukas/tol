#pragma once

#include <tile.hpp>

class Object: public Tile {
  std::reference_wrapper<const tson::Object> object;

  public:
  Object(const tson::Object& object, tson::Tile& tile, std::shared_ptr<AssetCache> asset_cache);

  bool intersects(const sf::FloatRect& rect) const;

  bool collides_with(const sf::FloatRect& rect) const;

  const std::string& getName() const;

  virtual std::optional<float> zIndex() const;
};
