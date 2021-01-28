#pragma once

#include <algorithm>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

#include <animation.hpp>

#if defined(_MSC_VER)
typedef long long ssize_t;
#endif

class TiledMap: public sf::Drawable {
  fs::path dir;
  fs::path filename;

  std::unique_ptr<tson::Map> map;
  sf::Vector2f scale = { 1.f, 1.f };

  mutable std::map<std::string, std::shared_ptr<const sf::Texture>> textures;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    for (auto& layer : map->getLayers()) {
      drawLayer(layer, target);
    }
  }

  std::shared_ptr<const sf::Texture> loadImage(const fs::path& path) const {
    if (textures.count(path.string()) == 0) {
      const auto absolute_path = dir / path;

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

  void drawLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    switch (layer.getType()) {

    case tson::LayerType::TileLayer:
      drawTileLayer(layer, target);
      break;
    case tson::LayerType::ObjectGroup:
      drawObjectLayer(layer, target);
      break;
    case tson::LayerType::ImageLayer:
      drawImageLayer(layer, target);
      break;
    case tson::LayerType::Group:
      for (auto& l : layer.getLayers()) {
        drawLayer(l, target);
      }
      break;
    default:
      break;
    }
  }

  mutable std::map<int, Animation> running_animations;

  void drawTileLayer(const tson::Layer& layer, sf::RenderTarget& target) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    for (const auto [pos, tileObject] : layer.getTileObjects()) {
      const auto* tile = tileObject.getTile();
      auto* tileset = tile->getTileset();

      tson::Rect tsonRect = tileObject.getTile()->getDrawingRect();
      sf::IntRect rect = { tsonRect.x, tsonRect.y, tsonRect.width, tsonRect.height };

      const auto& animation = tile->getAnimation();

      if (animation.size() > 0) {
        const auto tile_id = tile->getId();

        if (running_animations.count(tile_id) == 0) {
          std::cout << "Adding animation for tile " << tile_id << std::endl;
          running_animations.emplace(std::piecewise_construct, std::make_tuple(tile_id), std::make_tuple(animation, tileset));
        } else {
          rect = running_animations.at(tile_id).getDrawingRect(now);
        }

      }

      auto texture = loadImage(tileset->getImagePath());
      sf::Sprite sprite;
      sprite.setTexture(*texture);
      sprite.setTextureRect(rect);

      float rotation = sprite.getRotation();

      sf::Vector2f scale = sprite.getScale();
      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Horizontally))
        scale.x = -scale.x;
      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Vertically))
        scale.y = -scale.y;
      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Diagonally))
        rotation += 90.f;


      const auto& tile_position = tileObject.getPosition();
      sprite.setOrigin({ rect.width / 2.f, rect.height / 2.f });
      sf::Vector2f position = { tile_position.x + positionOffset.x, tile_position.y + positionOffset.y };
      sprite.setPosition({ position.x * this->scale.x, position.y * this->scale.y });

      sprite.setScale({ scale.x * this->scale.x, scale.y * this->scale.y });

      sprite.setRotation(rotation);

      target.draw(sprite);
    }
  }

  void drawImageLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    auto texture = loadImage(layer.getImage());
    sf::Sprite sprite;
    sprite.setTexture(*texture);
    sprite.setPosition({layer.getOffset().x, layer.getOffset().y});
    target.draw(sprite);
  }

  void drawObjectLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    auto* map = layer.getMap();
    for (auto& obj : layer.getObjects()) {
      switch (obj.getObjectType()) {
        case tson::ObjectType::Object: {
          auto* tileset = map->getTilesetByGid(obj.getGid());
          const auto offset = getTileOffset(obj.getGid(), map, tileset);

          auto texture = loadImage(tileset->getImagePath());
          sf::Sprite sprite;
          sprite.setTexture(*texture);
          std::string name = obj.getName();
          sf::Vector2f position = { (float)obj.getPosition().x + positionOffset.x, (float)obj.getPosition().y + positionOffset.y };

          sf::Vector2f scale = sprite.getScale();
          float rotation = sprite.getRotation();
          sf::Vector2f origin{ ((float) map->getTileSize().x) / 2, ((float) map->getTileSize().y) / 2 };

          if (obj.hasFlipFlags(tson::TileFlipFlags::Horizontally))
            scale.x = -scale.x;
          if (obj.hasFlipFlags(tson::TileFlipFlags::Vertically))
            scale.y = -scale.y;
          if (obj.hasFlipFlags(tson::TileFlipFlags::Diagonally))
            rotation += 90.f;

          position = { position.x + origin.x, position.y + origin.y };
          sprite.setOrigin(origin);

          sprite.setTextureRect({ (int)offset.x, (int)offset.y, map->getTileSize().x, map->getTileSize().y });
          sprite.setPosition({ position.x, position.y - map->getTileSize().y });

          sprite.setScale(scale);
          sprite.setRotation(rotation);

          target.draw(sprite);

          break;
        }
        default:
          break;
      }
    }
  }

  sf::Vector2f getTileOffset(const uint32_t tileId, const tson::Map* map, const tson::Tileset* tileset) const {
    uint32_t firstId = tileset->getFirstgid();
    int columns = tileset->getColumns();
    int rows = tileset->getTileCount() / columns;
    uint32_t lastId = (tileset->getFirstgid() + tileset->getTileCount()) - 1;

    if (tileId >= firstId && tileId <= lastId) {
      const size_t baseTilePosition = tileId - firstId;

      const ssize_t tileModX = (baseTilePosition % columns);
      const ssize_t currentRow = (baseTilePosition / columns);
      const ssize_t offsetX = (tileModX != 0) ? ((tileModX) * map->getTileSize().x) : (0 * map->getTileSize().x);
      const ssize_t offsetY = (currentRow < rows - 1) ? (currentRow * map->getTileSize().y) : ((rows - 1) * map->getTileSize().y);
      return sf::Vector2f((float) offsetX, (float) offsetY);
    }

    return { 0.f, 0.f };
  }

public:
  sf::Vector2f getSize() {
    auto [x, y] = map->getSize();
    auto [tile_size_x, tile_size_y] = map->getTileSize();
    return { static_cast<float>(x * tile_size_x), static_cast<float>(y * tile_size_y) };
  }

  sf::Vector2f positionOffset{ 0.f, 0.f };

  TiledMap(const fs::path& map_path) {
    dir = map_path.parent_path();
    filename = map_path.filename();

    tson::Tileson parser;
    map = parser.parse(map_path);

    if (map->getStatus() != tson::ParseStatus::OK) {
      throw std::runtime_error("Failed parsing '" + map_path.string() + "'.\n" + map->getStatusMessage());
    }
  }

  void set_position(const sf::Vector2f position, const sf::RenderTarget& target) {
    const auto tile_size = map->getTileSize();
    const auto x_offset = tile_size.x / 2.f;
    const auto y_offset = tile_size.y / 2.f;

    positionOffset = {
      std::clamp(position.x, -(getSize().x - (target.getSize().x / this->scale.x) - x_offset), x_offset),
      std::clamp(position.y, -(getSize().y - (target.getSize().y / this->scale.y) - y_offset), y_offset),
    };
  }

  void setScale(const sf::Vector2f scale) {
    this->scale = scale;
  }
};
