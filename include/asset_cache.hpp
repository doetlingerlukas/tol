#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>

#include <SFML/Graphics.hpp>
#include <nuklear.h>

namespace fs = std::filesystem;

class AssetCache {
  mutable std::map<std::string, std::shared_ptr<const std::vector<std::byte>>> bytes;
  mutable std::map<std::string, const nk_font*> nk_fonts;
  mutable std::map<std::string, std::shared_ptr<const sf::Font>> fonts;
  mutable std::map<std::string, std::shared_ptr<const sf::Texture>> textures;
  std::filesystem::path dir_;

public:
  AssetCache(std::filesystem::path dir): dir_(dir) {}

  const std::shared_ptr<const std::vector<std::byte>> loadFile(const fs::path& path) const {
    if (bytes.count(path.string()) == 0) {
      const auto absolute_path = dir_ / path;
      std::cout << "Loading " << absolute_path << std::endl;

      std::ifstream file(absolute_path, std::ios::binary);
      file.unsetf(std::ios::skipws);

      std::streampos fileSize;
      file.seekg(0, std::ios::end);
      fileSize = file.tellg();
      file.seekg(0, std::ios::beg);

      auto vec = std::make_shared<std::vector<std::byte>>(fileSize);
      file.read(reinterpret_cast<char*>(&vec->data()[0]), fileSize);

      bytes[path.string()] = std::move(vec);
    }

    return bytes.at(path.string());
  }

  const nk_font* loadNkFont(const fs::path& path, float size) const {
    if (nk_fonts.count(path.string()) == 0) {
      const auto& file = *loadFile(path);

      struct nk_font_atlas* atlas;
      nk_sfml_font_stash_begin(&atlas);
      struct nk_font* font = nk_font_atlas_add_from_memory(atlas, reinterpret_cast<void*>(const_cast<std::byte*>(file.data())), file.size(), size, nullptr);
      nk_sfml_font_stash_end();

      nk_fonts[path.string()] = font;

      if (font == nullptr) {
        const auto absolute_path = dir_ / path;
        throw std::runtime_error("Failed to load font '" + absolute_path.string() + "'.");
      }
    }

    return nk_fonts.at(path.string());
  }

  const std::shared_ptr<const sf::Font> loadFont(const fs::path& path) const {
    if (fonts.count(path.string()) == 0) {
      auto font = std::make_shared<sf::Font>();
      const auto& file = *loadFile(path);
      if (!font->loadFromMemory(file.data(), file.size())) {
        const auto absolute_path = dir_ / path;
        throw std::runtime_error("Failed to load font '" + absolute_path.string() + "'.");
      }

      fonts[path.string()] = std::move(font);
    }

    return fonts.at(path.string());
  }

  const std::shared_ptr<const sf::Texture> loadTexture(const fs::path& path) const {
    if (textures.count(path.string()) == 0) {
      auto texture = std::make_shared<sf::Texture>();
      const auto& file = *loadFile(path);
      if (!texture->loadFromMemory(file.data(), file.size())) {
        const auto absolute_path = dir_ / path;
        throw std::runtime_error("Failed to load texture '" + absolute_path.string() + "'.");
      }

      textures[path.string()] = std::move(texture);
      bytes[path.string()].reset(); // Remove texture from file cache.
    }

    return textures.at(path.string());
  }

  const std::filesystem::path& dir() const {
    return dir_;
  }
};
