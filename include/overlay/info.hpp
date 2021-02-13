#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

#include <asset_cache.hpp>


class Info : public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  std::chrono::milliseconds display_time;
  std::vector<std::string> words;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    if (display_time.count() > 0) {
      sf::Vector2f target_size({ (float)target.getSize().x, (float)target.getSize().y });
      sf::Vector2f info_box_size({ target_size.x * 0.35f, target_size.y * 0.125f });
      sf::Vector2f info_box_pos({ target_size.x - info_box_size.x - 5, 5 });

      sf::RectangleShape info_box;
      info_box.setSize(info_box_size);
      info_box.setFillColor(sf::Color(0, 0, 0, 100));
      info_box.setOutlineColor(sf::Color::Magenta);
      info_box.setOutlineThickness(2.f);
      info_box.setPosition(info_box_pos);
      target.draw(info_box);

      sf::Vector2f text_start_pos({ info_box_pos.x + 5, info_box_pos.y + 5 });
      auto max_line_width = info_box_size.x - 10;

      sf::Text text;
      text.setFont(*asset_cache->loadFont("fonts/Gaegu-Regular.ttf"));
      text.setColor(sf::Color::White);
      text.setPosition(text_start_pos);

      std::vector<sf::Text> lines;
      lines.push_back(text);
      for (auto& word : words) {
        if (lines.back().getGlobalBounds().width + word.size() * 4 > max_line_width) {
          lines.push_back(text);
          lines.back().setPosition({ text_start_pos.x, text_start_pos.y + 20 * lines.size() - 1 });
        }
        auto current = lines.back().getString();
        lines.back().setString(current + " " + word);
      }

      for (auto& line : lines) {
        target.draw(line);
      }
    }
  }

public:
  explicit Info(const std::shared_ptr<AssetCache> asset_cache_) : asset_cache(asset_cache_), display_time(0) {}

  void display_info(const std::string& text_, std::chrono::seconds duration) {
    display_time = std::chrono::milliseconds(duration);
    
    std::istringstream iss(text_);
    words = std::vector<std::string>({ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} });
  }

  void update_time(std::chrono::milliseconds delta) {
    display_time -= delta;
  }
};