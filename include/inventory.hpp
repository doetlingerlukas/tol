#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <asset_cache.hpp>
#include <collectibles.hpp>
#include <object.hpp>

class Protagonist;

class Inventory: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  int max_size;
  std::vector<std::pair<std::string, Object>> elements;

  sf::Vector2f mouse_location;
  bool mouse_pressed;
  mutable std::optional<int> selected;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override;

  public:
  Inventory(int max_size_, std::shared_ptr<AssetCache> asset_cache_);

  [[nodiscard]] int size() const;

  std::vector<std::pair<std::string, Object>> getElements() const;

  bool add(std::pair<std::string, Object> new_element);

  std::pair<std::string, Object> remove(size_t index);

  void mouse(sf::Vector2f location, bool pressed);

  void drop_selected();

  std::optional<std::string> use_selected(Protagonist& player);

  void select_next();
};

#ifndef TOL_PROTAGONIST_HPP
#include <protagonist.hpp>
#endif
