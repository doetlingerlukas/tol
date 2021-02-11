#pragma once

#include <character.hpp>

class Npc: public Character {
public:
  Npc(const fs::path& path, const std::shared_ptr<AssetCache> asset_cache, const std::shared_ptr<Stats> stats) : Character(path, asset_cache, stats) {}
};
