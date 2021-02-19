#include <fmt/core.h>

#include "protagonist.hpp"

namespace tol {

Protagonist::Protagonist(
  const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::shared_ptr<Stats> stats,
  const json& attack_json, const std::string& name):
  Character(path, asset_cache, stats, name, load_attacks(attack_json)),
  _inventory(Inventory(32, asset_cache)) {
  const auto file = asset_cache->load_file(fs::path("music/item-pick-up.ogg"));

  if (pick_up_sound_buffer.loadFromMemory(file->data(), file->size())) {
    pick_up_sound.setBuffer(pick_up_sound_buffer);
  } else {
    throw std::runtime_error("Failed loading pick up sound.");
  }
}

std::vector<sf::RectangleShape> Protagonist::move(
  std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction, float speed,
  std::chrono::milliseconds now, PlayState& play_state, std::map<int, Object>& collectibles,
  const sf::Vector2f& map_size, Info& info) {
  const auto shapes = Character::move(x_direction, y_direction, speed, now, play_state, map_size, info);

  if (now >= pick_up_allowed_after) {
    const auto bounds_ = bounds();
    for (auto it = collectibles.cbegin(); it != collectibles.cend();) {
      auto& [id, collectible] = *it;

      if (collectible.collides_with(bounds_)) {
        if (inventory().add(*it)) {
          pick_up_sound.play();
          info.display_info(fmt::format("Item collected: {}", collectible.name()), std::chrono::seconds(5));
          it = collectibles.erase(it);
          continue;
        } else {
          info.display_info("Inventory is full.", std::chrono::seconds(5));
        }
      }

      it++;
    }
  }

  return shapes;
}

std::vector<Attack> Protagonist::load_attacks(const json& attack_json) const {
  std::vector<Attack> att;

  for (const auto& n: attack_json) {
    att.push_back(Attack(n["name"], n["damage"]));
  }

  return att;
}

const Inventory& Protagonist::inventory() const {
  return _inventory;
}

Inventory& Protagonist::inventory() {
  return _inventory;
}

void Protagonist::drop_item() {
  pick_up_allowed_after = now + std::chrono::seconds(5);
}

std::optional<std::string> Protagonist::use_item(std::pair<int, Object> item) {
  auto [id, collectible] = item;
  std::cout << "Item used: " << collectible.name() << std::endl;
  const auto& effect = collectible.effect();
  if (effect) {
    (*effect)(*this);
  }

  return std::nullopt;
}

} // namespace tol
