#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include <asset_cache.hpp>


class Inventory : public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  std::vector<std::string> elements;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::Vector2f target_size({ (float)target.getSize().x, (float)target.getSize().y });
    sf::Vector2f inventory_size({ target_size.x * 0.7f, target_size.y * 0.7f });
    sf::Vector2f inventory_pos({ target_size.x * 0.15f, target_size.y * 0.2f });

    sf::RectangleShape inventory;
    inventory.setFillColor(sf::Color(0, 0, 0, 175));
    inventory.setSize(inventory_size);
    inventory.setOutlineColor(sf::Color::Blue);
    inventory.setOutlineThickness(2.f);
    inventory.setPosition(inventory_pos);

    target.draw(inventory);

    sf::Text text;
    text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
    text.setColor(sf::Color::White);
  }

public:
  explicit Inventory(const std::shared_ptr<AssetCache> asset_cache_) : asset_cache(asset_cache_) {
    elements = { "test1", "test2", "test3" };
  }
};