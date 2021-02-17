#pragma once

#include <nlohmann/json.hpp>

#include <character.hpp>

using json = nlohmann::json;

class Npc: public Character {
  inline static json load() {
    std::ifstream ifs("assets/character.json");
    json parsed = json::parse(ifs);
    return parsed["characters"];
  }

  inline static json npcs = load();

  json npc_stats(const std::string& name) const;

  std::vector<Attack> attacks(const std::string& name);

  public:
  Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::string& name);
};
