#pragma once

#include <character.hpp>
#include <nlohmann/json.hpp>

class Npc: public Character {
 using json = nlohmann::json;

  json npc_stats = {
    { "npc1", {
      { "strength", 10 },
      { "speed",  10 },
      { "level", 3 },
      { "health", 100 }
    }},
    { "npc2", {
        { "strength", 10 },
        { "speed", 10 },
        { "level",  3 },
        { "health", 100 },
    }}
  };

public:
  Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::string& name)
    : Character(path, asset_cache, std::make_shared<Stats>(npc_stats[name]), name) { }
};
