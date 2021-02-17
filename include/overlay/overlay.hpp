#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

#include <asset_cache.hpp>
#include <quest.hpp>

class Overlay: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;
  std::reference_wrapper<QuestStack> quest_stack;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::Vector2f target_size({ std::min((float)target.getSize().x, 1000.f), (float)target.getSize().y });
    sf::FloatRect overlay_dims(
      ((float)target.getSize().x - target_size.x) / 2, target_size.y * 0.05f, target_size.x, target_size.y * 0.9f);
    sf::FloatRect stats_rect(overlay_dims.left, overlay_dims.top, overlay_dims.width * 0.29f, overlay_dims.height);
    sf::FloatRect quests_rect(
      stats_rect.left + overlay_dims.width * 0.3f, overlay_dims.top, overlay_dims.width * 0.7f, overlay_dims.height);

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

    const auto font = *asset_cache->loadFont("fonts/Gaegu-Regular.ttf");
    auto height_offset = 0;

    const auto display_text = [&](std::string name_, std::string text_) {
      std::istringstream iss(text_);
      const std::vector<std::string> words(
        { std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} });

      sf::Text name;
      name.setFont(font);
      name.setStyle(sf::Text::Style::Bold);
      name.setFillColor(sf::Color::White);
      name.setPosition({ quests_rect.left + margin.x / 2, quests_rect.top + height_offset + margin.y / 2 });
      name.setString(" " + name_);
      target.draw(name);

      height_offset += name.getCharacterSize();
      sf::Text text;
      text.setFont(font);
      text.setFillColor(sf::Color::White);
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

    for (auto& quest: getQuestStack().quests) {
      display_text(quest->getName(), quest->getTask());
    }
  }

  public:
  explicit Overlay(const std::shared_ptr<AssetCache> asset_cache_, QuestStack& quest_stack_):
    asset_cache(asset_cache_), quest_stack(quest_stack_) {}

  inline QuestStack& getQuestStack() const {
    return quest_stack;
  }
};