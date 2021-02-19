#pragma once
#define TOL_INVENTORY_HPP

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "asset_cache.hpp"
#include "collectibles.hpp"
#include "object.hpp"

namespace tol {

class TiledMap;
class Protagonist;

class Inventory: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  size_t max_size;
  std::vector<std::pair<int, Object>> _items;

  sf::Vector2f mouse_location;
  bool mouse_pressed;
  mutable std::optional<int> selected;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  void select_next();

  public:
  Inventory(size_t max_size_, std::shared_ptr<AssetCache> asset_cache_);

  [[nodiscard]] bool empty() const;
  [[nodiscard]] int size() const;
  [[nodiscard]] const std::vector<std::pair<int, Object>>& items() const;
  bool add(std::pair<int, Object> item);
  std::pair<int, Object> remove(size_t index);

  void mouse(sf::Vector2f location, bool pressed);
  void drop_selected(Protagonist& player, TiledMap& map);
  std::optional<std::string> use_selected(Protagonist& player);
};

} // namespace tol
#ifndef TOL_MAP_HPP
#include <map.hpp>
#endif

#ifndef TOL_PROTAGONIST_HPP
#include <protagonist.hpp>
#endif
