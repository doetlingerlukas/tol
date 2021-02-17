#include <protagonist.hpp>

Protagonist::Protagonist(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::shared_ptr<Stats> stats,
    const std::string& name) : Character(path, asset_cache, stats, name, attacks()), inventory(Inventory(32)) {
  const auto file = asset_cache->loadFile(fs::path("music/item-pick-up.ogg"));

  if (pick_up_sound_buffer.loadFromMemory(file->data(), file->size())) {
    pick_up_sound.setBuffer(pick_up_sound_buffer);
  } else {
    throw std::runtime_error("Failed loading pick up sound.");
  }
}

std::vector<sf::RectangleShape> Protagonist::move(
  std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction,
  float speed, std::chrono::milliseconds now, PlayState& play_state, std::map<int, Object>& collectibles, const sf::Vector2f& map_size, Info& info
) {
  const auto shapes = Character::move(x_direction, y_direction, speed, now, play_state, map_size, info);

  const auto bounds = getBoundingRect();
  for (auto it = collectibles.cbegin(); it != collectibles.cend();) {
    auto& [id, collectible] = *it;

    if (collectible.collides_with(bounds)) {
      if (inventory.add(make_pair(collectible.getName(), collectible))) {
        pick_up_sound.play();
        std::cout << "Item collected: " << collectible.getName() << std::endl;

        const auto& found = collectible_effects.find(collectible.getName());
        if (found != collectible_effects.end()) {
          const auto& callback = found->second;
          callback();
        }

        it = collectibles.erase(it);
        continue;
      } else {
        std::cout << "Inventory is full." << std::endl;
      }
    }

    it++;
  }

  return shapes;
}

std::vector<Attack> Protagonist::attacks() const {
  return std::vector<Attack> {
    Attack("scratch", 5),
    Attack("holy birnbamm", 12),
    Attack("use spider", 32)
  };
}

std::vector<std::pair<std::string, Object>> Protagonist::getInventoryElements() const {
  return inventory.getElements();
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
