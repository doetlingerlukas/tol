#pragma once

#include <character.hpp>
#include <nlohmann/json.hpp>

class Npc: public Character {
 using json = nlohmann::json;

  json npc_stats(const std::string& name) const;

public:
  Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::string& name)
    : Character(path, asset_cache, std::make_shared<Stats>(npc_stats(name)), name) {
  }
};
