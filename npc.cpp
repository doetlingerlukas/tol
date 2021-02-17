#include <npc.hpp>

Npc::Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::string& name):
  Character(path, asset_cache, std::make_shared<Stats>(npc_stats(name)), name, attacks(name)) {}

json Npc::getNpcs() {
  std::ifstream ifs("assets/character.json");
  json parsed = json::parse(ifs);
  return parsed["characters"];
}

json Npc::npc_stats(const std::string& name) const {
  json npcs = Npc::getNpcs();
  return npcs[name];
}

std::vector<Attack> Npc::attacks(const std::string& name) {
  std::vector<Attack> att;
  json npcs = Npc::getNpcs();

  for (const auto& n: npcs[name]["attacks"]) {
    att.push_back(Attack(n["name"], n["damage"]));
  }

  return att;
}
