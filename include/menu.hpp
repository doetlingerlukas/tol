#pragma once

#include <functional>

class MenuItem: public sf::Drawable {
  std::shared_ptr<AssetCache> asset_cache;

  int character_scalar;
  std::string title;
  std::function<void()> callback_;
  sf::Vector2i menu_location;


  sf::Text text;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    target.draw(text);
  }

public:
  MenuItem(std::string title, std::function<void()> callback, const std::shared_ptr<AssetCache> asset_cache_, const sf::Vector2i& location,
      int character_scalar_) : character_scalar(character_scalar_), title(title), callback_(callback), asset_cache(asset_cache_), menu_location(location) {
    text.setString(title);
  }

  void update(size_t index, bool current, bool active, sf::Vector2f scale) {
    const auto character_size = character_scalar * scale.y;
    text.setCharacterSize(character_size);

    if (current) {
      text.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));

      text.setStyle(sf::Text::Bold);
      text.setFillColor(sf::Color::White);

      if (active) {
        text.setFillColor(sf::Color(50, 200, 100, 255));
      }
    } else {
      text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
      text.setFillColor(sf::Color(200, 200, 200, 255));
    }

    text.setPosition({ static_cast<float>(menu_location.x), static_cast<float>(index * character_size) + menu_location.y });
  }

  inline void callback() const {
    callback_();
  }

  inline sf::FloatRect global_bounds() const {
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

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    for (size_t i = 0; i < items.size(); i++) {
      items[i].update(i, i == current_item, enter_pressed || mouse_current_item_pressed, getScale());
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
  Menu(const std::shared_ptr<AssetCache> asset_cache_, int character_scalar_ = 32, sf::Vector2i location = {1, 1})
    : asset_cache(asset_cache_), character_scalar(character_scalar_), menu_location(location) {}

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

  void add_item(std::string title, std::function<void()> callback) {
    MenuItem item(title, callback, asset_cache, menu_location, character_scalar);
    items.push_back(item);
  }

  void enter(bool pressed) {
    const auto was_pressed = enter_pressed;
    enter_pressed = pressed;

    if (was_pressed && !pressed) {
      items[current_item].callback();
    }
  }

  void mouse(sf::Vector2i location, bool pressed) {
    const auto i = mouse_current_item(location);
    if (i) {
      current_item = *i;

      const auto was_pressed = mouse_current_item_pressed;
      mouse_current_item_pressed = pressed;

      if (was_pressed && !pressed) {
        items[current_item].callback();
      }
    } else {
      mouse_current_item_pressed = false;
    }
  }
};
