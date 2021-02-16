#include <npc.hpp>

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

