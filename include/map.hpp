#pragma once

#include <algorithm>
#include <chrono>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

#include <animation.hpp>

#if defined(_MSC_VER)
typedef long long ssize_t;
#endif

class TiledMap: public sf::Drawable, public sf::Transformable {
  fs::path dir;
  fs::path filename;

  std::unique_ptr<tson::Map> map;

  size_t from_x = 0;
  size_t to_x = 0;
  size_t from_y = 0;
  size_t to_y = 0;

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

  void drawTileLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    for (size_t x = from_x; x < to_x; x++) {
      for (size_t y = from_y; y < to_y; y++) {
        const auto* tileObjectP = layer.getTileObject(x, y);

        if (!tileObjectP) {
          continue;
        }

        const auto& tileObject = *tileObjectP;

        const auto& tile = *tileObject.getTile();
        auto* tileset = tile.getTileset();

        tson::Rect tsonRect = tile.getDrawingRect();
        sf::IntRect rect = { tsonRect.x, tsonRect.y, tsonRect.width, tsonRect.height };

        const auto& animation = tile.getAnimation();

        if (animation.size() > 0) {
          const auto tile_id = tile.getId();

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
        sf::Vector2f origin = { rect.width / 2.f, rect.height / 2.f };
        sprite.setOrigin(origin);
        sf::Vector2f position = { tile_position.x + getPosition().x, tile_position.y + getPosition().y };
        sprite.setPosition({ (origin.x + position.x) * getScale().x, (origin.y + position.y) * getScale().y });

        sprite.setScale({ scale.x * getScale().x, scale.y * getScale().y });

        sprite.setRotation(rotation);

        target.draw(sprite);
      }
    }

    for (const auto [pos, tileObject] : layer.getTileObjects()) {
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
          sf::Vector2f position = { (float)obj.getPosition().x + getPosition().x, (float)obj.getPosition().y + getPosition().y };

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
          sprite.setPosition({ position.x, position.y });

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
  sf::Vector2i getTileSize() const {
    auto [x, y] = map->getTileSize();
    return { x, y };
  }

  sf::Vector2f getSize() const {
    auto [x, y] = map->getSize();
    auto [tile_size_x, tile_size_y] = getTileSize();
    return { static_cast<float>(x * tile_size_x), static_cast<float>(y * tile_size_y) };
  }

  TiledMap(const fs::path& map_path) {
    dir = map_path.parent_path();
    filename = map_path.filename();

    tson::Tileson parser;
    map = parser.parse(map_path);

    if (map->getStatus() != tson::ParseStatus::OK) {
      throw std::runtime_error("Failed parsing '" + map_path.string() + "'.\n" + map->getStatusMessage());
    }

    for (const auto& tileset: map->getTilesets()) {
      loadImage(tileset.getImagePath());
    }
  }

  void update(size_t from_x_, size_t to_x_, size_t from_y_, size_t to_y_) {
    from_x = from_x_ / getScale().x / getTileSize().x;
    to_x = to_x_ / getScale().x / getTileSize().x + 1;
    from_y = from_y_ / getScale().y / getTileSize().y;
    to_y = to_y_ / getScale().y / getTileSize().y + 1;
  }

  void setPosition(sf::Vector2f position, const sf::RenderTarget& target) {
    position.x = std::clamp(position.x, -(getSize().x - target.getSize().x / getScale().x), 0.f);
    position.y = std::clamp(position.y, -(getSize().y - target.getSize().y / getScale().y), 0.f);

    sf::Transformable::setPosition(position);
  }

  std::optional<sf::Vector2f> getSpawn() {
    const auto& objects = map->getLayer("objects")->getObjects();

    const auto spawn = std::find_if(objects.begin(), objects.end(), [](const auto& object) {  return object.getType() == "spawn"; });

    if (spawn == objects.end()) {
      return std::nullopt;
    }

    return std::optional(sf::Vector2f({ (float)spawn->getPosition().x, (float)spawn->getPosition().y }));
  }
};
