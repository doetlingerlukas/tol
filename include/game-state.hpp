#pragma once

enum class GameState {
  MENU,
  PLAY,
  DIALOG,
  QUIT
};

// Represents an instance of a game with the current progress
class GameInstance {
  GameState state;

public:
  GameInstance() {
    state = GameState::MENU;
  }

  void setState(GameState new_state) {
    state = new_state;
  }

  GameState getState() {
    return state;
  }
};