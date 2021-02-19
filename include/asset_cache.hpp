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
#include <nuklear_sfml_gl2.h>

namespace tol {

namespace fs = std::filesystem;

class AssetCache {
  mutable std::map<std::string, std::shared_ptr<const std::vector<std::byte>>> bytes;
  mutable std::map<std::string, const nk_font*> nk_fonts;
  mutable std::map<std::string, std::shared_ptr<const sf::Font>> fonts;
  mutable std::map<std::string, std::shared_ptr<const sf::Texture>> textures;
  fs::path _dir;

  public:
  explicit AssetCache(fs::path dir);

  [[nodiscard]] std::shared_ptr<const std::vector<std::byte>> load_file(const fs::path& path) const;

  [[nodiscard]] const nk_font* load_nk_font(const fs::path& path, float size) const;

  [[nodiscard]] std::shared_ptr<const sf::Font> load_font(const fs::path& path) const;

  [[nodiscard]] std::shared_ptr<const sf::Texture> load_texture(const fs::path& path) const;

  [[nodiscard]] const fs::path& dir() const;
};

} // namespace tol
