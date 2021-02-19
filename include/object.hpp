#pragma once

#include "tile.hpp"

namespace tol {

class Object: public Tile {
  std::reference_wrapper<tson::Object> object;

  public:
  Object(tson::Object& object, tson::Tile& tile, std::shared_ptr<AssetCache> asset_cache);

  bool intersects(const sf::FloatRect& rect) const;

  bool collides_with(const sf::FloatRect& rect) const;

  const std::string& getName() const;

  virtual std::optional<float> zIndex() const override;

  bool usable();
};

} // namespace tol
