#pragma once

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

class TiledMap : public sf::Drawable {
  std::unique_ptr<tson::Map> map;
  std::map<std::string, std::unique_ptr<sf::Texture>> textures;
  std::map<std::string, std::unique_ptr<sf::Sprite>> sprites;
  tson::Vector2i positionOffset{ 0,0 };

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    for (auto& layer : map->getLayers()) {
      drawLayer(layer, target);
    }
  }

  bool parseMap(const std::string& map_path) {
    tson::Tileson parser;
    map = parser.parse(fs::path(map_path));

    if (map->getStatus() == tson::ParseStatus::OK) {
      for (auto& tileset : map->getTilesets()) {
        storeImage(tileset.getImage().filename().string(), { 0, 0 });
      }

      for (auto& layer : map->getLayers()) {
        if (!layer.getImage().empty()) {
          storeImage(layer.getImage(), { layer.getOffset().x, layer.getOffset().y });
        }
      }

      return true;
    }

    std::cerr << "Parse error: " << map->getStatusMessage() << std::endl;
    return false;
  }

  void storeImage(const std::string& image, const sf::Vector2f& position) {
    if (textures.count(image) == 0) {

      fs::path path = fs::path("assets") / image;
      if (fs::exists(path) && fs::is_regular_file(path)) {

        std::unique_ptr<sf::Texture> texture = std::make_unique<sf::Texture>();
        if (texture->loadFromFile(path.string())) {
          std::unique_ptr<sf::Sprite> sprite = std::make_unique<sf::Sprite>();
          sprite->setTexture(*texture);
          sprite->setPosition(position);
          textures[image] = std::move(texture);
          sprites[image] = std::move(sprite);
        }
      } else {
        std::cout << "Could not find: " << path.string() << std::endl;
      }
    }
  }

  sf::Sprite* loadImage(const std::string& image) const {
    if (sprites.count(image) > 0) {
      return sprites.at(image).get();
    }

    return nullptr;
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
      tson::Vector2f position = tileObject.getPosition();
      position = { position.x + (float) positionOffset.x, position.y + (float) positionOffset.y };
      
      const fs::path tilesetPath((*tileset).getImage().filename());
      sf::Sprite* sprite = loadImage(tilesetPath.string());

      if (sprite != nullptr) {
        sf::Vector2f scale = sprite->getScale();
        sf::Vector2f originalScale = scale;
        float rotation = sprite->getRotation();
        float originalRotation = rotation;
        sf::Vector2f origin{ ((float)drawingRect.width) / 2, ((float)drawingRect.height) / 2 };

        if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Horizontally))
          scale.x = -scale.x;
        if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Vertically))
          scale.y = -scale.y;
        if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Diagonally))
          rotation += 90.f;

        position = { position.x + origin.x, position.y + origin.y };
        sprite->setOrigin(origin);
        sprite->setTextureRect({ drawingRect.x, drawingRect.y, drawingRect.width, drawingRect.height });
        sprite->setPosition({ position.x, position.y });

        sprite->setScale(scale);
        sprite->setRotation(rotation);

        target.draw(*sprite);

        sprite->setScale(originalScale);
        sprite->setRotation(originalRotation);
      }
    }
  }

  void drawImageLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    sf::Sprite* sprite = loadImage(layer.getImage());
    if (sprite != nullptr) {
      target.draw(*sprite);
    }
  }

  void drawObjectLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    auto* map = layer.getMap();
    for (auto& obj : layer.getObjects()) {
      switch (obj.getObjectType()) {

      case tson::ObjectType::Object: {
        tson::Tileset* tileset = layer.getMap()->getTilesetByGid(obj.getGid());
        sf::Vector2f offset = getTileOffset(obj.getGid(), map, tileset);

        const fs::path tilesetPath((*tileset).getImage().filename());
        sf::Sprite* sprite = loadImage(tilesetPath.string());
        std::string name = obj.getName();
        sf::Vector2f position = { (float)obj.getPosition().x + (float) positionOffset.x, (float)obj.getPosition().y + (float) positionOffset.y };

        if (sprite != nullptr) {
          sf::Vector2f scale = sprite->getScale();
          sf::Vector2f originalScale = scale;
          float rotation = sprite->getRotation();
          float originalRotation = rotation;
          sf::Vector2f origin{ ((float) map->getTileSize().x) / 2, ((float) map->getTileSize().y) / 2 };

          if (obj.hasFlipFlags(tson::TileFlipFlags::Horizontally))
            scale.x = -scale.x;
          if (obj.hasFlipFlags(tson::TileFlipFlags::Vertically))
            scale.y = -scale.y;
          if (obj.hasFlipFlags(tson::TileFlipFlags::Diagonally))
            rotation += 90.f;

          position = { position.x + origin.x, position.y + origin.y };
          sprite->setOrigin(origin);

          sprite->setTextureRect({ (int)offset.x, (int)offset.y, map->getTileSize().x, map->getTileSize().y });
          sprite->setPosition({ position.x, position.y - map->getTileSize().y });

          sprite->setScale(scale);
          sprite->setRotation(rotation);

          target.draw(*sprite);

          sprite->setScale(originalScale);
          sprite->setRotation(originalRotation);
        }
      }
      break;

      default:
        break;
      }
    }
  }

  sf::Vector2f getTileOffset(int tileId, tson::Map* map, tson::Tileset* tileset) const {
    uint32_t firstId = tileset->getFirstgid();
    int columns = tileset->getColumns();
    int rows = tileset->getTileCount() / columns;
    uint32_t lastId = (tileset->getFirstgid() + tileset->getTileCount()) - 1;

    if (tileId >= firstId && tileId <= lastId) {
      uint32_t baseTilePosition = (tileId - firstId);

      int tileModX = (baseTilePosition % columns);
      int currentRow = (baseTilePosition / columns);
      int offsetX = (tileModX != 0) ? ((tileModX) *map->getTileSize().x) : (0 * map->getTileSize().x);
      int offsetY = (currentRow < rows - 1) ? (currentRow * map->getTileSize().y) : ((rows - 1) * map->getTileSize().y);
      return sf::Vector2f((float) offsetX, (float) offsetY);
    }

    return { 0.f, 0.f };
  }

public:

  TiledMap(const std::string& map_path) {
    parseMap(map_path);
  }

};