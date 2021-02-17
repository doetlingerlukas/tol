#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>

namespace fs = std::filesystem;

class AssetCache {
  mutable std::map<std::string, std::shared_ptr<const std::vector<std::byte>>> bytes;
  mutable std::map<std::string, const nk_font*> nk_fonts;
  mutable std::map<std::string, std::shared_ptr<const sf::Font>> fonts;
  mutable std::map<std::string, std::shared_ptr<const sf::Texture>> textures;
  fs::path dir_;

  public:
  AssetCache(fs::path dir);

  const std::shared_ptr<const std::vector<std::byte>> loadFile(const fs::path& path) const;

  const nk_font* loadNkFont(const fs::path& path, float size) const;

  const std::shared_ptr<const sf::Font> loadFont(const fs::path& path) const;

  const std::shared_ptr<const sf::Texture> loadTexture(const fs::path& path) const;

  const fs::path& dir() const;
};
