#pragma once

#include <chrono>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "asset_cache.hpp"

namespace tol {

class Info: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  std::chrono::milliseconds display_time;
  std::vector<std::string> words;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override {
    if (display_time.count() > 0) {
      sf::Vector2f target_size({ static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y) });
      sf::Vector2f info_box_size({ target_size.x * 0.35f, 0 });
      sf::Vector2f info_box_pos({ target_size.x - info_box_size.x - 5, 5 });

      sf::RectangleShape info_box;
      info_box.setFillColor(sf::Color(0, 0, 0, 175));
      info_box.setOutlineColor(sf::Color::Magenta);
      info_box.setOutlineThickness(2.f);
      info_box.setPosition(info_box_pos);

      sf::Vector2f text_start_pos({ info_box_pos.x + 5, info_box_pos.y + 2 });
      auto max_line_width = info_box_size.x - 10;

      sf::Text text;
      text.setFont(*asset_cache->load_font("fonts/Gaegu-Regular.ttf"));
      text.setFillColor(sf::Color::White);
      text.setPosition(text_start_pos);

      std::vector<sf::Text> lines;
      lines.push_back(text);
      for (auto& word: words) {
        if (lines.back().getGlobalBounds().width + word.size() * 14 > max_line_width) {
          lines.push_back(text);
          lines.back().setPosition(
            { text_start_pos.x, text_start_pos.y + text.getCharacterSize() * (lines.size() - 1) });
        }
        std::string current = lines.back().getString();
        lines.back().setString(current + " " + word);
      }

      info_box_size.y = static_cast<int>(text.getCharacterSize() * (lines.size() + 1));
      info_box.setSize(info_box_size);
      target.draw(info_box);

      for (auto& line: lines) {
        target.draw(line);
      }
    }
  }

  public:
  explicit Info(const std::shared_ptr<AssetCache> asset_cache_): asset_cache(asset_cache_), display_time(0) {}

  void display_info(const std::string& text_, std::chrono::seconds duration) {
    display_time = std::chrono::milliseconds(duration);

    std::istringstream iss(text_);
    words =
      std::vector<std::string>({ std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} });
  }

  void update_time(std::chrono::milliseconds delta) {
    display_time -= delta;
  }
};

} // namespace tol
