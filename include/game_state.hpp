#pragma once

#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include <play_state.hpp>
#include <settings.hpp>

using json = nlohmann::json;

namespace fs = std::filesystem;

enum class GameState {
  PLAY,
  DIALOG,
  FIGHT,
  QUEST,
  MENU,
  SETTINGS,
  QUIT
};

// Represents an instance of a game with the current progress
class GameInstance {
  GameState state;
  bool settings_changed;
  const fs::path saves_dir;

public:
  explicit GameInstance(const fs::path exec_dir) :
    state(GameState::MENU), settings_changed(false), saves_dir(exec_dir / "saves") {}

  void setState(GameState new_state) {
    state = new_state;
  }

  GameState getState() {
    return state;
  }

  bool isSettingsChanged() {
    return settings_changed;
  }

  void setSettingsChanged(bool value) {
    settings_changed = value;
  }

  void save(const PlayState& play_state) const {
    if (!fs::exists(saves_dir)) {
      fs::create_directory(saves_dir);
    }
  
    json save;
  
    auto player_pos = play_state.get_player_position();
    save["player"]["x"] = player_pos.x;
    save["player"]["y"] = player_pos.y;
  
    std::ofstream ofs(saves_dir / "game.json", std::ofstream::out | std::ofstream::trunc);
    ofs << save.dump(2);
    ofs.close();
  }
  
  void load(PlayState& play_state) {
    auto save_file = saves_dir / "game.json";
    json save;
  
    if (fs::exists(save_file)) {
      std::ifstream ifs(save_file);
      save = json::parse(ifs);
    } else {
      std::cout << "No save available at: " << save_file.string() << std::endl;
      return;
    }
  
    auto& player_j = save["player"];
  
    auto player_pos = play_state.get_player_position();
    const float x = get_or_else<float>(player_j, "x", player_pos.x);
    const float y = get_or_else<float>(player_j, "y", player_pos.y);
  
    play_state.set_player_position(sf::Vector2f(x, y));
  }
};