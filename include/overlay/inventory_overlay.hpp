#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <asset_cache.hpp>
#include <collectibles.hpp>
#include <inventory.hpp>
#include <object.hpp>

class InventoryOverlay: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  std::reference_wrapper<Inventory> inventory;

  sf::Vector2f mouse_location;
  bool mouse_pressed;

  mutable std::optional<int> selected;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    auto elements = getInventory().getElements();

    sf::Vector2f target_size({ std::min((float)target.getSize().x, 1000.f), (float)target.getSize().y });
    sf::FloatRect inventory_dims(
      ((float)target.getSize().x - target_size.x) / 2, target_size.y * 0.05f, target_size.x, target_size.y * 0.9f);
    sf::FloatRect detail(inventory_dims.left, inventory_dims.top, inventory_dims.width * 0.29f, inventory_dims.height);
    sf::FloatRect objects(
      detail.left + inventory_dims.width * 0.3f, inventory_dims.top, inventory_dims.width * 0.7f,
      inventory_dims.height);

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
    for (auto [name, element]: elements) {
      element.setScale(scale);

      auto rect = element.getBoundingRect();
      sf::Vector2f bounding_size({ rect.width * scale.x, rect.height * scale.y });
      sf::RectangleShape bounding_box(bounding_size);
      if (margin.x + w + bounding_size.x + margin.x > objects.width) {
        h += margin.y + bounding_size.y;
        w = 0;
      }
      bounding_box.setPosition({ objects.left + margin.x + w, objects.top + margin.y + h });
      if (bounding_box.getGlobalBounds().contains(mouse_location) && mouse_pressed) {
        selected = i;
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

    const auto font = *asset_cache->loadFont("fonts/Gaegu-Regular.ttf");
    const auto display_text = [&target, &font](const std::string& text_, sf::Vector2f pos) {
      sf::Text text;
      text.setFont(font);
      text.setFillColor(sf::Color::White);
      text.setString(text_);
      text.setPosition(pos);
      target.draw(text);
    };

    if (elements.size() > 0 && selected) {
      auto& [name, element] = elements.at(*selected);

      sf::Vector2f info_pos({ detail.left + margin.x / 2, detail.top + margin.y / 2 });
      auto max_line_width = detail.width - margin.x;

      display_text("<C> use item", { info_pos.x, detail.top + detail.height - 4 * margin.y });
      display_text("<X> drop item", { info_pos.x, detail.top + detail.height - 2 * margin.y });

      std::istringstream iss(getCollectible(name).info);
      const std::vector<std::string> words(
        { std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} });

      sf::Text text;
      text.setFont(font);
      text.setFillColor(sf::Color::White);
      text.setPosition(info_pos);

      std::vector<sf::Text> lines;
      lines.push_back(text);
      for (auto& word: words) {
        if (lines.back().getGlobalBounds().width + word.size() * 14 > max_line_width) {
          lines.push_back(text);
          lines.back().setPosition({ info_pos.x, info_pos.y + text.getCharacterSize() * (lines.size() - 1) });
        }
        std::string current = lines.back().getString();
        lines.back().setString(current + " " + word);
      }

      for (auto& line: lines) {
        target.draw(line);
      }
    }
  }

  public:
  explicit InventoryOverlay(const std::shared_ptr<AssetCache> asset_cache_, Inventory& inventory_):
    asset_cache(asset_cache_), inventory(inventory_) {}

  inline Inventory& getInventory() const {
    return inventory;
  }

  void mouse(sf::Vector2f location, bool pressed) {
    mouse_location = location;
    mouse_pressed = pressed;
  }

  void drop_selected() {
    if (selected) {
      getInventory().remove(*selected);
      select_next();
    }
  }

  void use_selected() {
    if (selected) {
      const auto [name, item] = getInventory().remove(*selected);
      std::cout << "Item used: " << name << std::endl;
      select_next();
    }
  }

  void select_next() {
    if (*selected < getInventory().size()) {
      return;
    } else if (*selected > 0) {
      (*selected)--;
    } else {
      selected = std::nullopt;
    }
  }
};
