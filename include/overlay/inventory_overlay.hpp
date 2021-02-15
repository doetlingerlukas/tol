#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <asset_cache.hpp>
#include <object.hpp>
#include <inventory.hpp>


class InventoryOverlay : public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  std::vector<std::pair<std::string, Object>> elements;

  sf::Vector2f mouse_location;
  bool mouse_pressed;

  mutable int selected;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::Vector2f target_size({ std::min((float)target.getSize().x, 1000.f), (float)target.getSize().y });
    sf::FloatRect inventory_dims(((float)target.getSize().x - target_size.x) / 2, target_size.y * 0.05f, target_size.x, target_size.y * 0.9f);
    sf::FloatRect detail(inventory_dims.left, inventory_dims.top, inventory_dims.width * 0.29f, inventory_dims.height);
    sf::FloatRect objects(detail.left + inventory_dims.width * 0.3f, inventory_dims.top, inventory_dims.width * 0.7f, inventory_dims.height);

    sf::RectangleShape detail_box;
    detail_box.setFillColor(sf::Color(0, 0, 0, 175));
    detail_box.setOutlineColor(sf::Color::Blue);
    detail_box.setOutlineThickness(2.f);
    detail_box.setSize({ detail.width, detail.height });
    detail_box.setPosition({ detail.left, detail.top });

    sf::RectangleShape objects_box;
    objects_box.setFillColor(sf::Color(0, 0, 0, 175));
    objects_box.setOutlineColor(sf::Color::Blue);
    objects_box.setOutlineThickness(2.f);
    objects_box.setSize({ objects.width, objects.height });
    objects_box.setPosition({ objects.left, objects.top });

    target.draw(detail_box);
    target.draw(objects_box);

    sf::Vector2f margin({ 30, 30 });
    sf::Vector2f scale({ 3.f, 3.f });

    auto i = 0;
    // Offsets for placement.
    auto h = 0;
    auto w = 0;
    for (auto [name, element] : elements) {
      element.setScale(scale);

      auto rect = element.getBoundingRect();
      sf::Vector2f bounding_size({ rect.width * scale.x, rect.height * scale.y });
      sf::RectangleShape bounding_box(bounding_size);
      bounding_box.setPosition({ objects.left + margin.x + w, objects.top + margin.y + h });
      if (bounding_box.getGlobalBounds().contains(mouse_location) && mouse_pressed) {
        selected = i;
      }
      if (margin.x + w + bounding_size.x + margin.x > objects.width) {
        h += margin.y + bounding_size.y;
        w = 0;
      }
      bounding_box.setFillColor(sf::Color(0, 0, 0, 220));
      bounding_box.setOutlineColor(i == selected ? sf::Color::White : sf::Color::Black);
      bounding_box.setOutlineThickness(2.f);
      target.draw(bounding_box);

      element.setPosition({ (objects.left + margin.x + w) / scale.x, (objects.top + margin.y + h) / scale.y });
      target.draw(element);

      w += margin.x + bounding_size.x;
      i++;
    }

    sf::Text text;
    text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
    text.setFillColor(sf::Color::White);
  }

public:
  explicit InventoryOverlay(const std::shared_ptr<AssetCache> asset_cache_) : asset_cache(asset_cache_), selected(0) {}

  void update_elements(std::vector<std::pair<std::string, Object>> elements_) {
    elements = elements_;
  }

  void mouse(sf::Vector2f location, bool pressed) {
    mouse_location = location;
    mouse_pressed = pressed;
  }
};
