#pragma once

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

class TiledMap : public sf::Drawable, public sf::Transformable {
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
        bool imageFound = texture->loadFromFile(path.string());
        if (imageFound) {
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

  sf::Sprite* loadImage(const std::string& image, const sf::Vector2f& position) const {
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
      //drawObjectLayer(layer);
      break;

    case tson::LayerType::ImageLayer:
      //drawImageLayer(layer);
      break;

    case tson::LayerType::Group:
      for (auto& l : layer.getLayers())
        drawLayer(l, target);
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
      
      fs::path tilesetPath((*tileset).getImage().filename());
      sf::Sprite* sprite = loadImage(tilesetPath.string(), { 0, 0 });

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

        sprite->setScale(originalScale);       //Since we used a shared sprite for this example, we must reset the scale.
        sprite->setRotation(originalRotation); //Since we used a shared sprite for this example, we must reset the rotation.
      }
    }
  }

public:

  TiledMap(const std::string& map_path) {
    parseMap(map_path);
  }

};