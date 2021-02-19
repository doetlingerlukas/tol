#pragma once

#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>

#include "asset_cache.hpp"
#include "quest.hpp"
#include "stats.hpp"

namespace tol {

class Overlay: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  std::reference_wrapper<const Stats> _stats;
  std::reference_wrapper<QuestStack> _quest_stack;

  sf::Vector2f mouse_location;
  bool mouse_pressed;

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override {
    sf::Vector2f target_size({ std::min((float)target.getSize().x, 1000.f), (float)target.getSize().y });
    sf::FloatRect overlay_dims(
      ((float)target.getSize().x - target_size.x) / 2, target_size.y * 0.1f, target_size.x, target_size.y * 0.85f);
    sf::FloatRect stats_rect(overlay_dims.left, overlay_dims.top, overlay_dims.width * 0.29f, overlay_dims.height);
    sf::FloatRect quests_rect(
      stats_rect.left + overlay_dims.width * 0.3f, overlay_dims.top, overlay_dims.width * 0.7f, overlay_dims.height);
    const auto font = *asset_cache->load_font("fonts/Gaegu-Regular.ttf");

    sf::Text header;
    header.setFont(font);
    header.setStyle(sf::Text::Style::Bold);
    header.setFillColor(sf::Color::White);
    header.setCharacterSize(60);
    header.setOutlineColor(sf::Color::Black);
    header.setOutlineThickness(4.f);
    header.setString("Stats & Quests");
    header.setPosition({ ((float)target.getSize().x - header.getGlobalBounds().width) / 2, 0 });
    target.draw(header);

    sf::RectangleShape stats_box;
    stats_box.setFillColor(sf::Color(0, 0, 0, 175));
    stats_box.setOutlineColor(sf::Color::Blue);
    stats_box.setOutlineThickness(2.f);
    stats_box.setSize({ stats_rect.width, stats_rect.height });
    stats_box.setPosition({ stats_rect.left, stats_rect.top });

    sf::RectangleShape quests_box;
    quests_box.setFillColor(sf::Color(0, 0, 0, 175));
    quests_box.setOutlineColor(sf::Color::Blue);
    quests_box.setOutlineThickness(2.f);
    quests_box.setSize({ quests_rect.width, quests_rect.height });
    quests_box.setPosition({ quests_rect.left, quests_rect.top });

    target.draw(stats_box);
    target.draw(quests_box);

    sf::Vector2f margin({ 30, 30 });

    auto height_offset = 0;

    const auto display_text = [&](std::string name_, std::string text_, int index) {
      std::istringstream iss(text_);
      const std::vector<std::string> words(
        { std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} });

      sf::Text name;
      name.setFont(font);
      name.setStyle(sf::Text::Style::Bold);
      if (quest_stack().getSelected() == index) {
        name.setFillColor(sf::Color::Green);
      } else {
        name.setFillColor(sf::Color::White);
      }
      name.setPosition({ quests_rect.left + margin.x / 2, quests_rect.top + height_offset + margin.y / 2 });
      name.setString(" " + name_);
      target.draw(name);

      auto bounds = name.getGlobalBounds();
      if (bounds.contains(mouse_location)) {
        quest_stack().select(index);
      }

      height_offset += name.getCharacterSize();
      sf::Text text;
      text.setFont(font);
      if (quest_stack().getSelected() == index) {
        text.setFillColor(sf::Color::Green);
      } else {
        text.setFillColor(sf::Color::White);
      }
      text.setPosition({ quests_rect.left + margin.x / 2, quests_rect.top + height_offset + margin.y / 2 });

      std::vector<sf::Text> lines;
      lines.push_back(text);
      for (auto& word: words) {
        if (lines.back().getGlobalBounds().width + word.size() * 14 > quests_rect.width - margin.x) {
          lines.push_back(text);
          height_offset += text.getCharacterSize();
          lines.back().setPosition({ quests_rect.left + margin.x / 2, quests_rect.top + height_offset + margin.y / 2 });
        }
        std::string current = lines.back().getString();
        lines.back().setString(current + " " + word);
      }
      height_offset += margin.y + text.getCharacterSize();

      for (auto& line: lines) {
        target.draw(line);
      }
    };

    auto i = 0;
    for (auto& quest: quest_stack().quests) {
      if (!quest.completed())
        display_text(quest.title(), quest.description(), i);
      i++;
    }

    auto stat_offset = 0;
    auto draw_stats_text = [&](std::string stat_) {
      sf::Text stat;
      stat.setFont(font);
      stat.setStyle(sf::Text::Style::Bold);
      stat.setFillColor(sf::Color::White);
      stat.setString(stat_);
      stat.setPosition({ stats_rect.left + margin.x / 2, stats_rect.top + stat_offset + margin.y / 2 });
      target.draw(stat);
      stat_offset += stat.getCharacterSize() + margin.y / 2;
    };

    auto ostreamable_to_string = [](auto& stream) {
      std::ostringstream oss;
      oss << stream;
      return oss.str();
    };

    auto exp = ostreamable_to_string(stats().experience());
    draw_stats_text(exp.substr(0, exp.find(", ")));
    exp.erase(0, exp.find(", ") + 2);
    draw_stats_text(exp);
    draw_stats_text(ostreamable_to_string(stats().strength()));
    draw_stats_text(ostreamable_to_string(stats().speed()));
    draw_stats_text(ostreamable_to_string(stats().health()));
  }

  inline const Stats& stats() const {
    return _stats;
  }

  public:
  explicit Overlay(const std::shared_ptr<AssetCache> asset_cache_, const Stats& stats, QuestStack& quest_stack):
    asset_cache(asset_cache_), _stats(stats), _quest_stack(quest_stack), mouse_pressed(false) {}

  inline QuestStack& quest_stack() const {
    return _quest_stack;
  }

  void mouse(sf::Vector2f location, bool pressed) {
    mouse_location = location;
    mouse_pressed = pressed;
  }
};

} // namespace tol
