#pragma once
#define TOL_PROTAGONIST_HPP

#include <map>
#include <optional>

#include <SFML/Audio.hpp>
#include <map>
#include <nlohmann/json.hpp>

#include "attack.hpp"
#include "character.hpp"
#include "game_state.hpp"
#include "inventory.hpp"

namespace tol {

using json = nlohmann::json;

class Protagonist: public Character {
  Inventory _inventory;

  sf::SoundBuffer pick_up_sound_buffer;
  sf::Sound pick_up_sound;

  std::map<std::string, std::function<std::optional<std::string>()>> collectible_effects{
    { "lemon",
      [&]() {
        stats->health().increase(30);
        return stats->get();
      } },
    { "strawberry",
      [&]() {
        stats->experience().increase(400);
        return stats->get();
      } },
    { "orange",
      [&]() {
        stats->strength().increase(1);
        return stats->get();
      } },
    { "melon",
      [&]() {
        stats->health().increase(50);
        return stats->get();
      } },
    { "pear",
      [&]() {
        stats->speed().increase(20);
        return stats->get();
      } },
    { "pistol",
      [&]() {
        addAttack(Attack("pistol", 20));
        return "Pistol available for fight.";
      } }
  };

  std::vector<Attack> attacks(const json& attack_json) const;

  std::set<std::string> talked_to_npcs;
  std::chrono::milliseconds pick_up_allowed_after = std::chrono::milliseconds(0);

  public:
  Protagonist(
    const fs::path& path, std::shared_ptr<AssetCache> asset_cache, std::shared_ptr<Stats> stats,
    const json& attack_json, const std::string& name);

  std::vector<sf::RectangleShape> move(
    std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed,
    std::chrono::milliseconds now, PlayState& play_state, std::map<int, Object>& collectibles,
    const sf::Vector2f& map_size, Info& info);

  const Inventory& inventory() const;
  Inventory& inventory();

  bool talked_to(const std::string& npc_name);

  void talk_to(const std::string& npc_name);

  void drop_item();
  std::optional<std::string> use_item(std::pair<int, Object> item);
};

} // namespace tol
