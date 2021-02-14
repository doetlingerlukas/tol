#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <iostream>

#include <asset_cache.hpp>
#include <object.hpp>


class Inventory : public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  std::map<int, Object> elements;

  // Sets the size of the inventory (size x size).
  const int size;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::Vector2f target_size({ (float)target.getSize().x, (float)target.getSize().y });
    sf::FloatRect inventory(target_size.x * 0.01f, target_size.y * 0.025f, target_size.x * 0.98f, target_size.y * 0.95f);
    sf::FloatRect detail(inventory.left, inventory.top, inventory.width * 0.29f, inventory.height);
    sf::FloatRect objects(detail.left + inventory.width * 0.3f, inventory.top, inventory.width * 0.7f, inventory.height);

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

    sf::Vector2f margin({ objects.width * 0.02f, objects.height * 0.02f });
    sf::Vector2f object_size({ (objects.width - (size + 1) * margin.x) / size , (objects.height - (size + 1) * margin.y) / size });

    auto i = 0;
    for (auto [key, element] : elements) {
      auto offset_x = i % size;
      auto offset_y = i / size;
      sf::RectangleShape bounding_box;
      bounding_box.setFillColor(sf::Color(0, 0, 0, 220));
      bounding_box.setSize(object_size);
      bounding_box.setOutlineColor(sf::Color::Blue);
      bounding_box.setOutlineThickness(2.f);
      bounding_box.setPosition({ margin.x + objects.left + offset_x * (object_size.x + margin.x), margin.y + objects.top + offset_y * (object_size.y + margin.y) });
      target.draw(bounding_box);

      element.setPosition({ margin.x + objects.left + offset_x * (object_size.x + margin.x), margin.y + objects.top + offset_y * (object_size.y + margin.y) });
      target.draw(element);
      i++;
    }

    sf::Text text;
    text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
    text.setColor(sf::Color::White);
  }

public:
  explicit Inventory(const std::shared_ptr<AssetCache> asset_cache_) : asset_cache(asset_cache_), size(5) {}

  void update_elements(std::map<int, Object> elements_) {
    elements = elements_;
  }
};