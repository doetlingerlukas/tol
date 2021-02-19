#include "asset_cache.hpp"

namespace tol {

AssetCache::AssetCache(fs::path dir): _dir(dir) {}

std::shared_ptr<const std::vector<std::byte>> AssetCache::load_file(const fs::path& path) const {
  if (bytes.count(path.string()) == 0) {
    const auto absolute_path = _dir / path;
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

const nk_font* AssetCache::load_nk_font(const fs::path& path, float size) const {
  if (nk_fonts.count(path.string()) == 0) {
    const auto& file = *load_file(path);

    struct nk_font_atlas* atlas;
    nk_sfml_font_stash_begin(&atlas);
    struct nk_font* font = nk_font_atlas_add_from_memory(
      atlas, reinterpret_cast<void*>(const_cast<std::byte*>(file.data())), file.size(), size, nullptr);
    nk_sfml_font_stash_end();

    nk_fonts[path.string()] = font;

    if (font == nullptr) {
      const auto absolute_path = _dir / path;
      throw std::runtime_error("Failed to load font '" + absolute_path.string() + "'.");
    }
  }

  return nk_fonts.at(path.string());
}

std::shared_ptr<const sf::Font> AssetCache::load_font(const fs::path& path) const {
  if (fonts.count(path.string()) == 0) {
    auto font = std::make_shared<sf::Font>();
    const auto& file = *load_file(path);
    if (!font->loadFromMemory(file.data(), file.size())) {
      const auto absolute_path = _dir / path;
      throw std::runtime_error("Failed to load font '" + absolute_path.string() + "'.");
    }

    fonts[path.string()] = std::move(font);
  }

  return fonts.at(path.string());
}

std::shared_ptr<const sf::Texture> AssetCache::load_texture(const fs::path& path) const {
  if (textures.count(path.string()) == 0) {
    auto texture = std::make_shared<sf::Texture>();
    const auto& file = *load_file(path);
    if (!texture->loadFromMemory(file.data(), file.size())) {
      const auto absolute_path = _dir / path;
      throw std::runtime_error("Failed to load texture '" + absolute_path.string() + "'.");
    }

    textures[path.string()] = std::move(texture);
    bytes[path.string()].reset(); // Remove texture from file cache.
  }

  return textures.at(path.string());
}

const fs::path& AssetCache::dir() const {
  return _dir;
}

} // namespace tol
