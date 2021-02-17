#include <game_instance.hpp>

GameInstance::GameInstance(const fs::path& exec_dir):
  state(GameState::MENU), settings_changed(false), saves_dir(exec_dir / "saves") {}

void GameInstance::setState(GameState new_state) {
  state = new_state;
}

GameState GameInstance::getState() {
  return state;
}

bool GameInstance::isSettingsChanged() {
  return settings_changed;
}

void GameInstance::setSettingsChanged(bool value) {
  settings_changed = value;
}

void GameInstance::save(const PlayState& play_state) const {
  if (!fs::exists(saves_dir)) {
    fs::create_directory(saves_dir);
  }

  json save;

  auto player_pos = play_state.player_position();
  save["player"]["x"] = player_pos.x;
  save["player"]["y"] = player_pos.y;

  std::ofstream ofs(saves_dir / "game.json", std::ofstream::out | std::ofstream::trunc);
  ofs << save.dump(2);
  ofs.close();
}

void GameInstance::load(PlayState& play_state) {
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

  auto player_pos = play_state.player_position();
  const float x = get_or_else<float>(player_j, "x", player_pos.x);
  const float y = get_or_else<float>(player_j, "y", player_pos.y);

  play_state.set_player_position(sf::Vector2f(x, y));
}
