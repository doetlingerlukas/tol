#include <npc.hpp>

Npc::Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::string& name)
  : Character(path, asset_cache, std::make_shared<Stats>(npc_stats(name)), name) {
}

json Npc::npc_stats(const std::string& name) const {
  json stats(json::object({
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
  }));

  return stats[name];
}

