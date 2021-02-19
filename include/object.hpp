#pragma once

#include <map>

#include "tile.hpp"

namespace tol {

class Object: public Tile {
  mutable std::reference_wrapper<tson::Object> object;

  inline static std::string UNDEFINED_DESCRIPTION = "Undefined object.";

  inline static std::map<std::string, std::string> DESCRIPTIONS{
    { "lemon", "A sour fruit. Can be used to gain 30 health." },
    { "strawberry", "Very sweet and delicious. Also adds 400 experience" },
    { "orange", "Makes a perfect juice. Increases strength by 1." },
    { "melon", "A tremendous watermelon. Eating it recovers 50 health." },
    { "pear", "Looks just like a regular fruit, but think twice before eating. Not everyone can handle the speed. +20 "
              "on walking speed" },
    { "cherry", "Probably edible." },
    { "pistol", "Watch out! This kills people." },
    { "tools", "I should probably bring these back to Detlef." },
    { "baguette", "Perfect for stabbing someone." }
  };

  public:
  Object(tson::Object& object, tson::Tile& tile, std::shared_ptr<AssetCache> asset_cache);

  bool intersects(const sf::FloatRect& rect) const;

  bool collides_with(const sf::FloatRect& rect) const;

  const std::string& name() const;

  const std::string& description() const;

  virtual std::optional<float> z_index() const override;

  bool usable() const;
};

} // namespace tol
