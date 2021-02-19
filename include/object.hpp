#pragma once
#define TOL_OBJECT_HPP

#include <map>
#include <optional>

#include "tile.hpp"

namespace tol {

class Protagonist;

class Object: public Tile {
  mutable std::reference_wrapper<tson::Object> object;

  static const std::map<std::string, std::string> DESCRIPTIONS;
  static const std::string UNDEFINED_DESCRIPTION;

  static const std::map<std::string, std::function<std::optional<std::string>(Protagonist& player)>> EFFECTS;

  public:
  Object(tson::Object& object, tson::Tile& tile, std::shared_ptr<AssetCache> asset_cache);

  bool intersects(const sf::FloatRect& rect) const;

  bool collides_with(const sf::FloatRect& rect) const;

  const std::string& name() const;

  const std::string& description() const;

  std::optional<std::function<std::optional<std::string>(Protagonist& player)>> effect() const;

  virtual std::optional<float> z_index() const override;

  bool usable() const;
};

} // namespace tol

#ifndef TOL_PROTAGONIST_HPP
#include "protagonist.hpp"
#endif
