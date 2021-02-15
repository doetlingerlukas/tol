#pragma once

#include "menu.hpp"
#include "input.hpp"
#include "protagonist.hpp"

class Fight: public sf::Drawable, public sf::Transformable {
  const Character& player;
  std::shared_ptr<AssetCache> asset_cache;
  Menu menu;
  std::chrono::milliseconds last_input = std::chrono::milliseconds(0);

  const int TILE_SIZE = 64;

  inline static std::function<int(float, float)> scale_ultra = [](const float x, const float y) constexpr {
    return (x / (x / y)) * 1.77777777778;
  };

  Menu init_menu(const std::pair<int, int>& resolution) {
    const auto& [x, y] = resolution;
    const int resize_x = Fight::scale_ultra(x, y);
    const int offset = x - resize_x + 200;
    return Menu(asset_cache, 52, { offset, y - 400 });
  }

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    sf::RectangleShape background;
    background.setSize({ (float)target.getSize().x, (float)target.getSize().y });
    background.setFillColor(sf::Color(132, 94, 28, 200));
    target.draw(background);

    const int scale_factor = 5;

    const int resize_x = Fight::scale_ultra(target.getSize().x, target.getSize().y);

    sf::Sprite player_sprite;
    player_sprite.setTexture(*asset_cache->loadTexture("tilesets/character-whitebeard.png"));
    player_sprite.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 3, TILE_SIZE, TILE_SIZE });
    player_sprite.setPosition({ target.getSize().x - resize_x - 100.0f, target.getSize().y - (100.0f * scale_factor) });
    player_sprite.setScale({ scale_factor, scale_factor });
    target.draw(player_sprite);

    auto health = player.getStats()->health().get();

    sf::RectangleShape health_background;
    health_background.setSize({ 440.0f, 30.0f });
    health_background.setPosition({ static_cast<float>(target.getSize().x - resize_x), target.getSize().y - 150.0f });
    health_background.setFillColor(sf::Color(255, 232, 225, 100));
    target.draw(health_background);

    sf::RectangleShape health_fill;
    health_fill.setSize({ 440.0f * (health / 100.0f), 30.0f });
    health_fill.setPosition({ static_cast<float>(target.getSize().x - resize_x), target.getSize().y - 150.0f });

    if (health > 40 && health < 70) {
      health_fill.setFillColor(sf::Color(255, 165, 0, 255));
    } else if (health >= 70) {
      health_fill.setFillColor(sf::Color(36, 109, 36, 255));
    } else {
      health_fill.setFillColor(sf::Color(255, 0, 0, 255));
    }

    target.draw(health_fill);

    sf::Text player_health_percent;
    player_health_percent.setCharacterSize(22);
    player_health_percent.setFillColor(sf::Color::White);
    player_health_percent.setString(fmt::format("{}", health));
    player_health_percent.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
    player_health_percent.setPosition({ target.getSize().x - resize_x + 200.0f, target.getSize().y - 152.0f });
    target.draw(player_health_percent);

    sf::Sprite enemy;

    enemy.setTexture(*asset_cache->loadTexture("tilesets/character-whitebeard.png"));
    enemy.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 5, TILE_SIZE, TILE_SIZE });
    enemy.setPosition({ resize_x - 100.0f, 100.0f });
    enemy.setScale({ scale_factor, scale_factor });
    target.draw(enemy);

    target.draw(menu);
  }

public:
  Fight(const std::shared_ptr<AssetCache> asset_cache_, const Character& player_, const std::pair<int, int>& resolution) :
      player(player_), asset_cache(asset_cache_), menu(init_menu(resolution)) {
    menu.add_item("ATTACK 1", [&]() { });
    menu.add_item("ATTACK 2", [&]() { });
    menu.add_item("ATTACK 3", [&]() { });
  }

  void with(const KeyInput& input, std::chrono::milliseconds now) {
    const auto td = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_input);
    const auto stats = player.getStats();

    if(input.up) {
      if(td.count() > 120) {
        menu.up();
        last_input = now;
      }
    } else if(input.down) {
      if(td.count() > 120) {
        menu.down();
        last_input = now;
      }
    } else if(input.enter) {
      if(td.count() > 120) {
        menu.enter(input.enter);
        last_input = now;
      }
    } else {
      menu.enter(false);
    }

  }
};
