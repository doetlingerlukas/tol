#pragma once

#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include <play_state.hpp>
#include <settings.hpp>

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

  bool isSettingsChanged();

  void setSettingsChanged(bool value);

  void save(const PlayState& play_state) const;

  void load(PlayState& play_state);
};
