#pragma once

#include <algorithm>

#include "menu.hpp"
#include "input.hpp"
#include "protagonist.hpp"
#include "shared.hpp"
#include "map.hpp"

enum class Turn {
  PLAYER,
  ENEMY
};

class Fight: public sf::Drawable, public sf::Transformable {
  const Character& player;
  const Character* npc = nullptr;
  Turn fight_turn = Turn::PLAYER;

  std::shared_ptr<AssetCache> asset_cache;
  Menu menu;
  std::chrono::milliseconds now = std::chrono::milliseconds(0);
  std::chrono::milliseconds last_input = std::chrono::milliseconds(0);
  std::chrono::milliseconds last_turn = std::chrono::milliseconds(0);

  const int TILE_SIZE = 64;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {

    sf::RectangleShape background;
    background.setSize({ (float)target.getSize().x, (float)target.getSize().y });
    background.setFillColor(sf::Color(132, 94, 28, 200));
    target.draw(background);

    const int scale_factor = 5;

    const int resize_x = scale_ultra(target.getSize().x, target.getSize().y);

    sf::Sprite player_sprite;
    player_sprite.setTexture(*asset_cache->loadTexture(player.getCharacterTexture()));
    player_sprite.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 3, TILE_SIZE, TILE_SIZE });
    player_sprite.setPosition({ target.getSize().x - resize_x + scale_pos(100.0f, target.getSize().x, resize_x, -200.0f), target.getSize().y - (100.0f * scale_factor) });
    player_sprite.setScale({ scale_factor, scale_factor });
    target.draw(player_sprite);

    auto health = player.getStats()->health().get();

    const float health_bar_off = 170.0f;
    const float health_bar_size = 440.0f;

    sf::RectangleShape health_background;
    health_background.setSize({ health_bar_size, 30.0f });
    health_background.setPosition({ target.getSize().x - resize_x + scale_pos(health_bar_off, target.getSize().x, resize_x, -150.0f), target.getSize().y - 150.0f });
    health_background.setFillColor(sf::Color(255, 232, 225, 100));
    target.draw(health_background);

    sf::RectangleShape health_fill;
    health_fill.setSize({ 440.0f * (health / 100.0f), 30.0f });
    health_fill.setPosition({ target.getSize().x - resize_x + scale_pos(health_bar_off, target.getSize().x, resize_x, -150.0f), target.getSize().y - 150.0f });


    const int character_size = 22;
    sf::Text player_health_percent;
    player_health_percent.setCharacterSize(character_size);
    player_health_percent.setFillColor(sf::Color::White);
    player_health_percent.setString(fmt::format("{}", health));
    player_health_percent.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));

    const auto health_text_pos = health_bar_size / 2 - character_size;

    player_health_percent.setPosition({
      target.getSize().x - resize_x + scale_pos(health_text_pos + health_bar_off, target.getSize().x, resize_x, 55.0f),
      target.getSize().y - 152.0f
    });


    sf::Sprite enemy;

    const auto enemy_health = npc->getStats()->health().get();

    enemy.setTexture(*asset_cache->loadTexture(npc->getCharacterTexture()));
    enemy.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 5, TILE_SIZE, TILE_SIZE });
    enemy.setPosition({ resize_x - scale_pos(400.0f, target.getSize().x, resize_x, 0.0f), 100.0f });
    enemy.setScale({ scale_factor, scale_factor });
    target.draw(enemy);

    sf::RectangleShape health_enemy_background;
    health_enemy_background.setSize({ health_bar_size, 30.0f });
    health_enemy_background.setPosition({ resize_x - scale_pos(400.0f + health_bar_off, target.getSize().x, resize_x, 150.0f), 100.0f });
    health_enemy_background.setFillColor(sf::Color(255, 232, 225, 100));
    target.draw(health_enemy_background);

    sf::RectangleShape health_enemy_fill;
    health_enemy_fill.setSize({ health_bar_size * (enemy_health / 100.0f), 30.0f });
    health_enemy_fill.setPosition({ resize_x - scale_pos(400.0f + health_bar_off, target.getSize().x, resize_x, 150.0f), 100.0f });

    if (health > 40 && health < 70) {
      health_fill.setFillColor(sf::Color(255, 165, 0, 255));
      health_enemy_fill.setFillColor(sf::Color(255, 165, 0, 255));
    } else if (health >= 70) {
      health_fill.setFillColor(sf::Color(36, 109, 36, 255));
      health_enemy_fill.setFillColor(sf::Color(36, 109, 36, 255));
    } else {
      health_fill.setFillColor(sf::Color(255, 0, 0, 255));
      health_enemy_fill.setFillColor(sf::Color(255, 0, 0, 255));
    }

    target.draw(health_fill);
    target.draw(health_enemy_fill);

    sf::Text enemy_health_percent;
    enemy_health_percent.setCharacterSize(character_size);
    enemy_health_percent.setFillColor(sf::Color::White);
    enemy_health_percent.setString(fmt::format("{}", enemy_health));
    enemy_health_percent.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));

    enemy_health_percent.setPosition({
      resize_x - scale_pos(400 / 2 + health_bar_off, target.getSize().x, resize_x, -55.0f),
      98.0f
    });

    target.draw(enemy_health_percent);

    sf::Text enemy_name;
    enemy_name.setCharacterSize(40);
    enemy_name.setFillColor(sf::Color::White);
    enemy_name.setString(npc->getName());
    enemy_name.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
    enemy_name.setPosition({ resize_x - scale_pos(400.0f + health_bar_off, target.getSize().x, resize_x, 150.0f), 20.0f });
    target.draw(enemy_name);

    const auto enemy_level_text = fmt::format("Level: {}", npc->getStats()->experience().getLevel());

    sf::Text enemy_level;
    enemy_level.setCharacterSize(40);
    enemy_level.setFillColor(sf::Color::White);
    enemy_level.setString(enemy_level_text);
    enemy_level.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
    const int enemy_text_size = enemy_level.getGlobalBounds().width;
    enemy_level.setPosition({ resize_x - scale_pos(enemy_text_size * 2, target.getSize().x, resize_x, -enemy_text_size), 20.0f });

    target.draw(enemy_level);
    target.draw(player_health_percent);

    target.draw(menu);
  }

public:
  Fight(const std::shared_ptr<AssetCache> asset_cache_, const Character& player_) :
      player(player_), asset_cache(asset_cache_), menu(Menu(asset_cache, 52, { 340, 370 })) {

    const auto& attacks = player.getAttacks();

    for (const auto& attack : attacks) {
      menu.add_item(attack.getName(), [attacks, this](int idx) mutable {
        auto damage = attacks[idx].getDamage();
        npc->getStats()->health().decrease(damage);
        fight_turn = Turn::ENEMY;
        last_turn = this->now;
      });
    }
  }

  void with(const KeyInput& input, std::chrono::milliseconds now_, const std::optional<std::string>& npc_interact, const TiledMap& map) {
    now = now_;
    const auto td = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_input);

    if(npc_interact && npc == nullptr) {
      const auto& characters = map.getCharacters();
      const auto& res = std::find_if(characters.cbegin(), characters.cend(), [&npc_interact](const auto& c) {
        return c->getName() == *npc_interact;
      });

      if(res != std::end(characters))
        npc = *res;
    }

    assert(npc != nullptr);

    if(fight_turn == Turn::PLAYER) {
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
    } else {
      const auto td_turn = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_turn);

      if(td_turn.count() > 1000) {
        fight_turn = Turn::PLAYER;
        player.getStats()->health().decrease(10);
      }
    }
  }
};
