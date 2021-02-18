#include "game_instance.hpp"

namespace tol {

GameInstance::GameInstance(const fs::path& exec_dir):
  state(GameState::MENU), settings_changed(false), saves_dir(exec_dir / "saves") {}

void GameInstance::setState(GameState new_state) {
  state = new_state;
}

GameState GameInstance::getState() {
  return state;
}

bool GameInstance::isSettingsChanged() const {
  return settings_changed;
}

void GameInstance::setSettingsChanged(bool value) {
  settings_changed = value;
}

void GameInstance::save(const PlayState& play_state, const Character& player, const Inventory& inventory) const {
  if (!fs::exists(saves_dir)) {
    fs::create_directory(saves_dir);
  }

  json save;

  auto player_pos = play_state.player_position();
  save["player"]["position"]["x"] = player_pos.x;
  save["player"]["position"]["y"] = player_pos.y;

  auto& stats = save["player"]["stats"];

  const auto& player_stats = player.getStats();

  stats["health"] = player_stats->health().get();
  stats["speed"] = player_stats->speed().get();
  stats["experience"] = player_stats->experience().get();
  stats["level"] = player_stats->experience().getLevel();
  stats["strength"] = player_stats->strength().get();

  auto& attacks = save["player"]["attacks"];

  const auto& player_attacks = player.getAttacks();

  for(const auto& attack: player_attacks) {
    attacks[attack.getName()] = attack.getDamage();
  }

  auto& inventory_array = save["player"]["inventory"] = json::array();

  const auto& inventory_elements = inventory.getElements();

  for(const auto& [id, obj]: inventory_elements) {
    inventory_array.push_back(id);
  }

  std::ofstream ofs(saves_dir / "game.json", std::ofstream::out | std::ofstream::trunc);
  ofs << save.dump(2);
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

} // namespace tol
