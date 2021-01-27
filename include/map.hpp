#pragma once

#include <algorithm>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

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

  void drawTileLayer(const tson::Layer& layer, sf::RenderTarget& target) const {
    for (const auto& [pos, tileObject] : layer.getTileObjects()) {
      tson::Tileset* tileset = tileObject.getTile()->getTileset();
      tson::Rect drawingRect = tileObject.getDrawingRect();
      sf::Vector2f origin{ 0, 0 };

      auto texture = loadImage(tileset->getImagePath());
      sf::Sprite sprite;
      sprite.setTexture(*texture);

      sf::Vector2f scale = sprite.getScale();

      auto tile_position = tileObject.getPosition();
      sf::Vector2f position = { tile_position.x + positionOffset.x, tile_position.y + positionOffset.y };

      float rotation = sprite.getRotation();

      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Horizontally))
        scale.x = -scale.x;
      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Vertically))
        scale.y = -scale.y;
      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Diagonally))
        rotation += 90.f;

      sprite.setTextureRect({ drawingRect.x, drawingRect.y, drawingRect.width, drawingRect.height });

      sprite.setOrigin({ origin.x * this->scale.x, origin.y * this->scale.y });
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

  sf::Vector2f getTileOffset(const int tileId, const tson::Map* map, const tson::Tileset* tileset) const {
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
    positionOffset = {
      std::clamp(position.x, -(getSize().x - target.getSize().x / this->scale.x), 0.f),
      std::clamp(position.y, -(getSize().y - target.getSize().y / this->scale.y), 0.f),
    };
  }

  void setScale(const sf::Vector2f scale) {
    this->scale = scale;
  }
};
