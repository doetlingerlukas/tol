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

  std::vector<Attack> load_attacks(const json& attack_json) const;

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

  void drop_item();
  std::optional<std::string> use_item(std::pair<int, Object> item);
};

} // namespace tol
