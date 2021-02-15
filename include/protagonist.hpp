#pragma once

#include <SFML/Audio.hpp>
#include <map>

#include <character.hpp>

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

  json protagonist_stats = {
    { "strength", 10 },
    { "speed",  10 },
    { "health", 100 },
    { "level",  1 }
  };

public:
  Protagonist(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::shared_ptr<Stats> stats,
      const std::string& name) : Character(path, asset_cache, stats, name), inventory(Inventory(32)) {
    const auto file = asset_cache->loadFile(fs::path("music/item-pick-up.ogg"));

    if (pick_up_sound_buffer.loadFromMemory(file->data(), file->size())) {
      pick_up_sound.setBuffer(pick_up_sound_buffer);
    } else {
      throw std::runtime_error("Failed loading pick up sound.");
    }
  }

  void move(
    std::optional<CharacterDirection> x_direction, std::optional<CharacterDirection> y_direction,
    float speed, std::chrono::milliseconds now, std::vector<sf::RectangleShape>& collision_rects, std::map<int, Object>& collectibles, const sf::Vector2f& map_size
  ) {
    Character::move(x_direction, y_direction, speed, now, collision_rects, map_size);

    const auto bounds = getBoundingRect();
    for (auto it = collectibles.cbegin(); it != collectibles.cend();) {
      auto& [id, collectible] = *it;

      if (collectible.collides_with(bounds)) {
        std::cout << "Item collected: " << collectible.getName() << std::endl;
        inventory.add(make_pair(collectible.getName(), collectible));
        pick_up_sound.play();

        const auto& found = collectible_effects.find(collectible.getName());
        if (found != collectible_effects.end()) {
          const auto& callback = found->second;
          callback();
        }

        it = collectibles.erase(it);
      } else {
        it++;
      }
    }
  }

  std::vector<std::pair<std::string, Object>> getInventoryElements() const {
    return inventory.getElements();
  }

};
