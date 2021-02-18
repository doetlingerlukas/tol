#pragma once

#include <nlohmann/json.hpp>

#include "character.hpp"

namespace tol {

using json = nlohmann::json;

class Npc: public Character {
  static json load() {
    std::ifstream ifs("assets/character.json");
    json parsed = json::parse(ifs);
    return parsed["characters"];
  }

  static json getNpcs();

  json npc_stats(const std::string& name) const;

  std::vector<Attack> attacks(const std::string& name);

  public:
  Npc(const fs::path& path, std::shared_ptr<AssetCache> asset_cache, const std::string& name);
};

} // namespace tol
