#pragma once

#include <filesystem>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "play_state.hpp"
#include "settings.hpp"

namespace tol {

using json = nlohmann::json;

namespace fs = std::filesystem;

// Represents an instance of a game with the current progress
class GameInstance {
  GameState state;
  bool settings_changed;
  const fs::path saves_dir;

  json init() const;

  json player_default_attacks = json::array({
      json::object({
        {"name", "scratch" },
        {"damage", 5 }
      }),
      json::object({
        {"name", "kick" },
        {"damage", 12 }
      }),
      json::object({
        {"name", "use spider" },
        {"damage", 16 }
      })
    });

  json player_default_stats = json::object({
    { "strength", 10 },
    { "speed", 10 },
    { "health", 100 },
    { "level", 1 }
  });

  public:
  explicit GameInstance(const fs::path& exec_dir);

  void setState(GameState new_state);

  GameState getState();

  [[nodiscard]] bool isSettingsChanged() const;

  void setSettingsChanged(bool value);

  void save(const PlayState& play_state, const Character& player, const Inventory& inventory, const QuestStack& quests) const;

  void load_position(PlayState& play_state);

  json load_attacks() const;
  json load_stats() const;
  json load_inventory() const;
  json load_quests() const;
};

} // namespace tol
