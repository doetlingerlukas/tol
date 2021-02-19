#include "game_instance.hpp"

namespace tol {

GameInstance::GameInstance(const fs::path& exec_dir):
  _state(GameState::PLAY), settings_changed(false), saves_dir(exec_dir / "saves") {}

void GameInstance::set_state(GameState state) {
  _state = state;
}

GameState GameInstance::state() {
  return _state;
}

bool GameInstance::isSettingsChanged() const {
  return settings_changed;
}

void GameInstance::set_settings_changed(bool value) {
  settings_changed = value;
}

json GameInstance::init() const {
  auto save_file = saves_dir / "game.json";
  std::ifstream ifs(save_file);

  json save;

  try {
    save = json::parse(ifs);
  } catch ([[maybe_unused]] std::exception& ex) {
    save = json::object({ { "player",
                            { { "position", { { "x", 543 }, { "y", 2845 } } },
                              { "stats", player_default_stats },
                              { "attacks", player_default_attacks },
                              { "quests", { { "completed", nullptr }, { "active", nullptr } } },
                              { "inventory", nullptr },
                              { "useditems", nullptr } } } });
  }

  return save;
}

void GameInstance::remove() {
  auto save_file = saves_dir / "game.json";
  fs::remove(save_file);
}

void GameInstance::save(const PlayState& play_state) const {
  if (!fs::exists(saves_dir)) {
    fs::create_directory(saves_dir);
  }

  json save;

  const auto& player = play_state.player();

  auto player_pos = player.getPosition();
  save["player"]["position"]["x"] = player_pos.x;
  save["player"]["position"]["y"] = player_pos.y;

  auto& stats = save["player"]["stats"];

  const auto& player_stats = player.stats();
  stats["health"] = player_stats.health().get();
  stats["speed"] = player_stats.speed().get();
  stats["experience"] = player_stats.experience().get();
  stats["strength"] = player_stats.strength().get();

  auto& attacks = save["player"]["attacks"] = json::array();

  const auto& player_attacks = player.attacks();

  for (const auto& attack: player_attacks) {
    attacks.push_back(json::object({ { "name", attack.getName() }, { "damage", attack.getDamage() } }));
  }

  auto& inventory_array = save["player"]["inventory"] = json::array();

  const auto& inventory_items = player.inventory().items();

  for (const auto& [id, obj]: inventory_items) {
    inventory_array.push_back(id);
  }

  auto& used_items = save["player"]["useditems"] = json::array();
  for (const auto id: play_state.used_collectibles()) {
    used_items.push_back(id);
  }

  auto& quests_json = save["player"]["quests"];

  auto& completed_quests = quests_json["completed"] = json::array();

  const auto& quests = play_state.quest_stack();
  for (size_t i = 0; i < quests.count(); i++) {
    if (quests.completed(i)) {
      completed_quests.push_back(i);
    }
  }

  quests_json["active"] = quests.getSelected();

  std::ofstream ofs(saves_dir / "game.json", std::ofstream::out | std::ofstream::trunc);
  ofs << save.dump(2);
}

void GameInstance::load(PlayState& play_state) {
  auto& quest_stack = play_state.quest_stack();

  for (size_t quest_id: load_quests()) {
    quest_stack.quests[quest_id].setCompleted();
  }

  const auto& active_quest = load_active_quest();

  if (!active_quest.is_null()) {
    quest_stack.select(active_quest);
  }

  load_position(play_state);
  play_state.set_inventory(load_inventory());
  play_state.set_stats(load_stats());
  play_state.set_attacks(load_attacks());
  play_state.set_used_collectibles(load_used_items());
}

json GameInstance::load_attacks() const {
  json save = init();
  const auto& player = save["player"];
  return get_or_else(player, "attacks", player_default_attacks);
}

json GameInstance::load_stats() const {
  json save = init();
  const auto& player = save["player"];
  return get_or_else(player, "stats", player_default_stats);
}

void GameInstance::load_position(PlayState& play_state) {
  json save = init();

  auto& player_j = save["player"]["position"];

  auto& player = play_state.player();
  auto player_pos = player.getPosition();

  const float x = get_or_else<float>(player_j, "x", player_pos.x);
  const float y = get_or_else<float>(player_j, "y", player_pos.y);

  player.setPosition(sf::Vector2f(x, y));
}

json GameInstance::load_inventory() const {
  json save = init();
  return get_or_else(save["player"], "inventory", json::array());
}

json GameInstance::load_used_items() const {
  json save = init();
  return get_or_else(save["player"], "useditems", json::array());
}

json GameInstance::load_quests() const {
  json save = init();
  return get_or_else(save["player"]["quests"], "completed", json::array());
}

json GameInstance::load_active_quest() const {
  json save = init();
  return get_or_else(save["player"]["quests"], "active", json::object());
}

} // namespace tol
