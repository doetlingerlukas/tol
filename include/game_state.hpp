#pragma once

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

public:
  GameInstance() : state(GameState::MENU), settings_changed(false) {}

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
};