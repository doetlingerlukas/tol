#pragma once
#define _PROTAGONIST_HPP_

#include <SFML/Audio.hpp>
#include <map>

#include <character.hpp>
#include <game_state.hpp>
#include <attack.hpp>

class Protagonist: public Character {
  Inventory inventory;

  sf::SoundBuffer pick_up_sound_buffer;
  sf::Sound pick_up_sound;

  std::map<std::string, std::function<void()>> collectible_effects {
    {"lemon", [&]() {
      stats->health().increase(30);
      stats->get();
    }},
    {"strawberry", [&]() {
      stats->experience().increase(400);
      stats->get();
    }},
    {"orange", [&]() {
      stats->strength().increase(1);
      stats->get();
    }},
    {"melon", [&]() {
      stats->health().increase(50);
      stats->get();
    }},
    {"pear", [&]() {
      stats->speed().increase(20);
      stats->get();
    }}
  };

  std::vector<Attack> attacks() const;

  std::set<std::string> talked_to_npcs;

public:
  Protagonist(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::shared_ptr<Stats> stats,
      const std::string& name);

  std::vector<sf::RectangleShape> move(
    std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction,
    float speed, std::chrono::milliseconds now, PlayState& play_state, std::map<int, Object>& collectibles, const sf::Vector2f& map_size, Info& info
  );

  std::vector<std::pair<std::string, Object>> getInventoryElements() const;

  Inventory& getInventory();

  bool talked_to(const std::string& npc_name);

  void talk_to(const std::string& npc_name);
};
