#include <fmt/core.h>

#include "protagonist.hpp"

namespace tol {

Protagonist::Protagonist(
  const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::shared_ptr<Stats> stats,
  const json& attack_json, const std::string& name):
  Character(path, asset_cache, stats, name, attacks(attack_json)),
  inventory(Inventory(32, asset_cache)) {
  const auto file = asset_cache->loadFile(fs::path("music/item-pick-up.ogg"));

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
    const auto bounds = getBoundingRect();
    for (auto it = collectibles.cbegin(); it != collectibles.cend();) {
      auto& [id, collectible] = *it;

      if (collectible.collides_with(bounds)) {
        if (inventory.add(*it)) {
          pick_up_sound.play();
          info.display_info(fmt::format("Item collected: {}", collectible.getName()), std::chrono::seconds(5));
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

std::vector<Attack> Protagonist::attacks(const json& attack_json) const {
  std::vector<Attack> att;

  for (const auto& n: attack_json) {
    att.push_back(Attack(n["name"], n["damage"]));
  }

  return att;
}

const std::vector<std::pair<int, Object>>& Protagonist::getInventoryElements() const {
  return inventory.items();
}

Inventory& Protagonist::getInventory() {
  return inventory;
}

bool Protagonist::talked_to(const std::string& npc_name) {
  return talked_to_npcs.count(npc_name) > 0;
}

void Protagonist::talk_to(const std::string& npc_name) {
  talked_to_npcs.insert(npc_name);
}

void Protagonist::drop_item() {
  pick_up_allowed_after = now + std::chrono::seconds(5);
}

std::optional<std::string> Protagonist::use_item(std::pair<int, Object> item) {
  auto [id, collectible] = item;
  std::cout << "Item used: " << collectible.getName() << std::endl;
  const auto& found = collectible_effects.find(collectible.getName());
  if (found != collectible_effects.end()) {
    const auto& callback = found->second;
    return callback();
  }

  return std::nullopt;
}

} // namespace tol
