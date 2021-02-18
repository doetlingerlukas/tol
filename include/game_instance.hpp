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

  public:
  explicit GameInstance(const fs::path& exec_dir);

  void setState(GameState new_state);

  GameState getState();

  [[nodiscard]] bool isSettingsChanged() const;

  void setSettingsChanged(bool value);

  void save(const PlayState& play_state, const Character& player, const Inventory& inventory) const;

  void load_position(PlayState& play_state);

  json load_attacks() const;
  json load_stats() const;
};

} // namespace tol
