#pragma once

#include <SFML/Graphics.hpp>

class AssetCache {
  mutable std::map<std::string, std::shared_ptr<const sf::Texture>> textures;
  std::filesystem::path dir_;

public:
  AssetCache(std::filesystem::path dir): dir_(dir) {}

  const std::shared_ptr<const sf::Texture> loadTexture(const fs::path& path) const {
    if (textures.count(path.string()) == 0) {
      const auto absolute_path = dir_ / path;

      if (fs::exists(absolute_path) && fs::is_regular_file(absolute_path)) {
        std::cout << "Loading " << absolute_path << std::endl;

        auto texture = std::make_shared<sf::Texture>();
        if (texture->loadFromFile(absolute_path.string())) {
          textures[path.string()] = std::move(texture);
          return textures.at(path.string());
        }
      }

      throw std::runtime_error("Failed to load '" + path.string() + "'.");
    } else {
      return textures.at(path.string());
    }
  }

  const std::filesystem::path& dir() const {
    return dir_;
  }
};
