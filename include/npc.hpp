#pragma once

#include <nlohmann/json.hpp>

#include <character.hpp>

class Npc: public Character {
  using json = nlohmann::json;

  json npc_stats(const std::string& name) const;

public:
  Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::string& name);
};
