#pragma once

#include <functional>

#include "shared.hpp"

namespace tol {

class MenuItem: public sf::Drawable {
  friend class Menu;

  int character_scalar;
  std::string title;

  std::shared_ptr<AssetCache> asset_cache;

  std::function<void(int)> _callback;

  sf::Text text;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override {
    target.draw(text);
  }

  sf::Vector2i menu_location;

  public:
  MenuItem(
    std::string title, std::function<void(int)> callback, const std::shared_ptr<AssetCache> asset_cache_,
    const sf::Vector2i& location, int character_scalar_):
    character_scalar(character_scalar_),
    title(title), asset_cache(asset_cache_), _callback(callback), menu_location(location) {
    text.setString(title);
  }

  void update(size_t index, bool current, bool active, sf::Vector2f scale, const int x, const int y) {
    const auto character_size = character_scalar * scale.y;
    text.setCharacterSize(character_size);

    if (current) {
      text.setFont(*asset_cache->load_font("fonts/Gaegu-Bold.ttf"));

      text.setStyle(sf::Text::Bold);
      text.setFillColor(sf::Color::White);

      if (active) {
        text.setFillColor(sf::Color(50, 200, 100, 255));
      }
    } else {
      text.setFont(*asset_cache->load_font("fonts/Gaegu-Regular.ttf"));
      text.setFillColor(sf::Color(200, 200, 200, 255));
    }

    const int resize_x = scale_ultra(x, y);
    const auto ultra_offset = scale_pos(0.0f, x, resize_x, 300.0f);
    const auto offset_x = x - resize_x + static_cast<float>(menu_location.x) - ultra_offset;
    const int offset_y = y - menu_location.y;

    text.setPosition({ offset_x, offset_y + static_cast<float>(index * character_size) });
  }

  inline void callback(int idx) const {
    _callback(idx);
  }

  [[nodiscard]] inline sf::FloatRect global_bounds() const {
    return text.getGlobalBounds();
  }
};

class Menu: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  size_t current_item = 0;
  mutable std::vector<MenuItem> items;
  bool enter_pressed = false;
  bool mouse_current_item_pressed = false;
  int character_scalar;
  sf::Vector2i menu_location;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override {
    for (size_t i = 0; i < items.size(); i++) {
      items[i].update(
        i, i == current_item, enter_pressed || mouse_current_item_pressed, getScale(), target.getSize().x,
        target.getSize().y);
      target.draw(items[i]);
    }
  }

  std::optional<size_t> mouse_current_item(sf::Vector2i location) {
    for (size_t i = 0; i < items.size(); i++) {
      const auto& item = items[i];
      if (item.global_bounds().contains(location.x, location.y)) {
        return i;
      }
    }

    return std::nullopt;
  }

  public:
  explicit Menu(
    const std::shared_ptr<AssetCache> asset_cache_, int character_scalar_ = 32, sf::Vector2i location = { 1, 1 }):
    asset_cache(asset_cache_),
    character_scalar(character_scalar_), menu_location(location) {}

  void up() {
    if (current_item > 0) {
      current_item--;
    }
  }

  void down() {
    if (current_item + 1 < items.size()) {
      current_item++;
    }
  }

  void add_item(std::string title, std::function<void(int)> callback) {
    MenuItem item(title, callback, asset_cache, menu_location, character_scalar);
    items.push_back(item);
  }

  void reposition(sf::Vector2i new_location) {
    menu_location = new_location;

    for (auto& item: items) {
      item.menu_location = new_location;
    }
  }

  void enter(bool pressed) {
    const auto was_pressed = enter_pressed;
    enter_pressed = pressed;

    if (was_pressed && !pressed) {
      items[current_item].callback(current_item);
    }
  }

  [[nodiscard]] size_t count() const {
    return items.size();
  }

  [[nodiscard]] sf::Vector2i location() const {
    return menu_location;
  }

  void mouse(sf::Vector2i location, bool pressed) {
    const auto i = mouse_current_item(location);
    if (i) {
      current_item = *i;

      const auto was_pressed = mouse_current_item_pressed;
      mouse_current_item_pressed = pressed;

      if (was_pressed && !pressed) {
        items[current_item].callback(current_item);
      }
    } else {
      mouse_current_item_pressed = false;
    }
  }
};

} // namespace tol
