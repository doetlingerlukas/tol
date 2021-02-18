#pragma once

#include <algorithm>
#include <ctime>

#include "game_state.hpp"
#include "input.hpp"
#include "map.hpp"
#include "menu.hpp"
#include "protagonist.hpp"
#include "shared.hpp"

enum class Turn { PLAYER, ENEMY };

class Fight: public sf::Drawable, public sf::Transformable {
  const Character& player;
  const Character* npc = nullptr;
  Turn fight_turn = Turn::PLAYER;

  std::shared_ptr<AssetCache> asset_cache;
  Menu menu;
  std::chrono::milliseconds now = std::chrono::milliseconds(0);
  std::chrono::milliseconds last_turn = std::chrono::milliseconds(0);

  std::optional<int> last_enemy_damage;
  std::optional<std::string> last_enemy_attack;

  std::optional<int> last_player_damage;
  std::optional<std::string> last_player_attack;

  const int TILE_SIZE = 64;

  inline static std::function<int(int, int, int)> calcDamage =
    [](const int level, const int damage, const int strength) {
      return std::ceil(damage * (level / 7.5f + 1) - (strength / 10.0f) * 1.05f);
    };

  void draw(sf::RenderTarget& target, sf::RenderStates state) const override {
    sf::RectangleShape background;
    background.setSize({ static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y) });
    background.setFillColor(sf::Color(132, 94, 28, 200));
    target.draw(background);

    const int scale_factor = 5;

    const int resize_x = scale_ultra(target.getSize().x, target.getSize().y);

    sf::Sprite player_sprite;
    player_sprite.setTexture(*asset_cache->loadTexture(player.getCharacterTexture()));
    player_sprite.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 3, TILE_SIZE, TILE_SIZE });
    player_sprite.setPosition(
      { target.getSize().x - resize_x + scale_pos(100.0f, target.getSize().x, resize_x, -200.0f),
        target.getSize().y - (100.0f * scale_factor) });
    player_sprite.setScale({ scale_factor, scale_factor });
    target.draw(player_sprite);

    auto health = player.getStats()->health().get();

    const float health_bar_off = 170.0f;
    const float health_bar_size = 440.0f;

    sf::RectangleShape health_background;
    health_background.setSize({ health_bar_size, 30.0f });
    health_background.setPosition(
      { target.getSize().x - resize_x + scale_pos(health_bar_off, target.getSize().x, resize_x, -150.0f),
        target.getSize().y - 150.0f });
    health_background.setFillColor(sf::Color(255, 232, 225, 100));
    target.draw(health_background);

    sf::RectangleShape health_fill;
    health_fill.setSize({ 440.0f * (health / 100.0f), 30.0f });
    health_fill.setPosition(
      { target.getSize().x - resize_x + scale_pos(health_bar_off, target.getSize().x, resize_x, -150.0f),
        target.getSize().y - 150.0f });

    const int character_size = 22;
    sf::Text player_health_percent;
    player_health_percent.setCharacterSize(character_size);
    player_health_percent.setFillColor(sf::Color::White);
    player_health_percent.setString(fmt::format("{}", health));
    player_health_percent.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));

    const auto health_text_pos = health_bar_size / 2 - character_size;

    player_health_percent.setPosition(
      { target.getSize().x - resize_x +
          scale_pos(health_text_pos + health_bar_off, target.getSize().x, resize_x, 55.0f),
        target.getSize().y - 152.0f });

    sf::Sprite enemy;

    const auto enemy_health = npc->getStats()->health().get();

    enemy.setTexture(*asset_cache->loadTexture(npc->getCharacterTexture()));
    enemy.setTextureRect(sf::IntRect{ TILE_SIZE, TILE_SIZE * 5, TILE_SIZE, TILE_SIZE });
    enemy.setPosition({ resize_x - scale_pos(400.0f, target.getSize().x, resize_x, 0.0f), 100.0f });
    enemy.setScale({ scale_factor, scale_factor });
    target.draw(enemy);

    sf::RectangleShape health_enemy_background;
    health_enemy_background.setSize({ health_bar_size, 30.0f });
    health_enemy_background.setPosition(
      { resize_x - scale_pos(400.0f + health_bar_off, target.getSize().x, resize_x, 150.0f), 100.0f });
    health_enemy_background.setFillColor(sf::Color(255, 232, 225, 100));
    target.draw(health_enemy_background);

    sf::RectangleShape health_enemy_fill;
    health_enemy_fill.setSize({ health_bar_size * (enemy_health / 100.0f), 30.0f });
    health_enemy_fill.setPosition(
      { resize_x - scale_pos(400.0f + health_bar_off, target.getSize().x, resize_x, 150.0f), 100.0f });

    if (health > 40 && health < 70) {
      health_fill.setFillColor(sf::Color(255, 165, 0, 255));
    } else if (health >= 70) {
      health_fill.setFillColor(sf::Color(36, 109, 36, 255));
    } else {
      health_fill.setFillColor(sf::Color(255, 0, 0, 255));
    }

    if (enemy_health > 40 && enemy_health < 70) {
      health_enemy_fill.setFillColor(sf::Color(255, 165, 0, 255));
    } else if (enemy_health >= 70) {
      health_enemy_fill.setFillColor(sf::Color(36, 109, 36, 255));
    } else {
      health_enemy_fill.setFillColor(sf::Color(255, 0, 0, 255));
    }

    target.draw(health_fill);
    target.draw(health_enemy_fill);

    sf::Text enemy_health_percent;
    enemy_health_percent.setCharacterSize(character_size);
    enemy_health_percent.setFillColor(sf::Color::White);
    enemy_health_percent.setString(fmt::format("{}", enemy_health));
    enemy_health_percent.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));

    enemy_health_percent.setPosition(
      { resize_x - scale_pos(400 / 2 + health_bar_off, target.getSize().x, resize_x, -55.0f), 98.0f });

    target.draw(enemy_health_percent);

    sf::Text enemy_name;
    enemy_name.setCharacterSize(40);
    enemy_name.setFillColor(sf::Color::White);
    enemy_name.setString(npc->getName());
    enemy_name.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
    enemy_name.setPosition(
      { resize_x - scale_pos(400.0f + health_bar_off, target.getSize().x, resize_x, 150.0f), 20.0f });
    target.draw(enemy_name);

    const auto enemy_level_text = fmt::format("Level: {}", npc->getStats()->experience().getLevel());

    sf::Text enemy_level;
    enemy_level.setCharacterSize(40);
    enemy_level.setFillColor(sf::Color::White);
    enemy_level.setString(enemy_level_text);
    enemy_level.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
    const int enemy_text_size = enemy_level.getGlobalBounds().width;
    enemy_level.setPosition(
      { resize_x - scale_pos(enemy_text_size * 2, target.getSize().x, resize_x, -enemy_text_size), 20.0f });

    if (last_enemy_attack && last_enemy_damage) {
      sf::Text attack_info;
      attack_info.setCharacterSize(70);
      attack_info.setFillColor(sf::Color::White);
      attack_info.setString(
        fmt::format("{} used \"{}\" for {} damage.", npc->getName(), *last_enemy_attack, *last_enemy_damage));
      attack_info.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
      attack_info.setPosition(
        { target.getSize().x / 2.0f - attack_info.getGlobalBounds().width / 2.0f, target.getSize().y / 2.0f - 35.0f });
      target.draw(attack_info);
    }

    if (last_player_attack && last_player_damage) {
      sf::Text attack_info;
      attack_info.setCharacterSize(70);
      attack_info.setFillColor(sf::Color::White);
      attack_info.setString(fmt::format("Detlef used \"{}\" for {} damage.", *last_player_attack, *last_player_damage));
      attack_info.setFont(*asset_cache->loadFont("fonts/Gaegu-Bold.ttf"));
      attack_info.setPosition(
        { target.getSize().x / 2.0f - attack_info.getGlobalBounds().width / 2.0f, target.getSize().y / 2.0f - 35.0f });
      target.draw(attack_info);
    }

    target.draw(enemy_level);
    target.draw(player_health_percent);

    if (fight_turn == Turn::PLAYER) {
      target.draw(menu);
    }
  }

  void resetFight() {
    npc = nullptr;
    last_enemy_attack = std::nullopt;
    last_enemy_damage = std::nullopt;
    last_player_attack = std::nullopt;
    last_player_damage = std::nullopt;
    fight_turn = Turn::PLAYER;
    last_turn = std::chrono::milliseconds(0);
  }

  public:
  Fight(const std::shared_ptr<AssetCache> asset_cache_, const Character& player_):
    player(player_), asset_cache(asset_cache_), menu(Menu(asset_cache, 52, { 340, 370 })) {
    std::srand(std::time(nullptr));

    const auto& attacks = player.getAttacks();

    for (const auto& attack: attacks) {
      menu.add_item(attack.getName(), [attacks, this](int idx) mutable {
        auto damage = attacks[idx].getDamage();
        const auto player_level = player.getStats()->experience().getLevel();
        const auto& npc_stats = npc->getStats();
        const auto npc_strength = npc_stats->strength().get();

        auto final_damage = calcDamage(player_level, damage, npc_strength);
        npc_stats->health().decrease(final_damage);
        fight_turn = Turn::ENEMY;
        last_turn = this->now;
        last_enemy_attack = std::nullopt;
        last_enemy_damage = std::nullopt;
        last_player_attack = std::make_optional(attacks[idx].getName());
        last_player_damage = std::make_optional(final_damage);
      });
    }
  }

  void up() {
    if (fight_turn == Turn::PLAYER) {
      menu.up();
    }
  }

  void down() {
    if (fight_turn == Turn::PLAYER) {
      menu.down();
    }
  }

  void enter(bool pressed) {
    if (fight_turn == Turn::PLAYER) {
      menu.enter(pressed);
    }
  }

  GameState with(std::chrono::milliseconds now_, const std::optional<std::string>& npc_interact, const TiledMap& map) {
    now = now_;

    if (player.getStats()->health().get() == 0) {
      resetFight();
      return GameState::DEAD;
    }

    if (npc != nullptr && npc->getStats()->health().get() == 0) {
      resetFight();
      return GameState::PLAY;
    }

    if (npc_interact && npc == nullptr) {
      const auto& characters = map.getCharacters();
      const auto& res = std::find_if(characters.cbegin(), characters.cend(), [&npc_interact](const auto& c) {
        return c->getName() == *npc_interact;
      });

      if (res != std::end(characters))
        npc = *res;
    }

    assert(npc != nullptr);

    if (fight_turn == Turn::ENEMY) {
      if (now > last_turn + std::chrono::milliseconds(3000)) {
        fight_turn = Turn::PLAYER;
        const auto& attacks = npc->getAttacks();
        const auto& npc_stats = npc->getStats();
        const auto npc_level = npc_stats->experience().getLevel();
        const auto player_strength = player.getStats()->strength().get();

        int rnd = std::rand() / ((RAND_MAX + 1u) / attacks.size());
        const auto& enemy_attack = attacks[rnd];

        const auto attack_damage = calcDamage(npc_level, enemy_attack.getDamage(), player_strength);
        last_enemy_attack = std::make_optional(enemy_attack.getName());
        last_enemy_damage = std::make_optional(attack_damage);
        last_player_attack = std::nullopt;
        last_player_damage = std::nullopt;

        player.getStats()->health().decrease(*last_enemy_damage);
      }
    }

    return GameState::FIGHT;
  }
};
