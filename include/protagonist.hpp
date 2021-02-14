#pragma once

#include <SFML/Audio.hpp>
#include <map>

#include <character.hpp>

class Protagonist: public Character {
  sf::SoundBuffer pick_up_sound_buffer;
  sf::Sound pick_up_sound;

  std::map<std::string, std::function<void()>> collectibles {
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
      const std::string& name) : Character(path, asset_cache, stats, name) {
    const auto file = asset_cache->loadFile(fs::path("music/item-pick-up.ogg"));

    if (!pick_up_sound_buffer.loadFromMemory(file->data(), file->size())) {
      throw std::runtime_error("Failed loading sound.");
    }

    pick_up_sound.setBuffer(pick_up_sound_buffer);

    registerPickup([&](const std::string& collectible) {
      pick_up_sound.play();

      auto found = collectibles.find(collectible);

      if (found != collectibles.end()) {
        found->second();
      }
    });
  }
};
