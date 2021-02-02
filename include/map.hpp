#pragma once

#include <variant>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

#include <asset_cache.hpp>
#include <tile.hpp>
#include <animation.hpp>
#include <character.hpp>

class TiledMap: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  fs::path dir;
  fs::path filename;

  std::unique_ptr<tson::Map> map;

  size_t from_x;
  size_t to_x;
  size_t from_y;
  size_t to_y;

  const Character* character;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    std::vector<std::variant<Tile, Character>> deferred_tiles;

    for (auto& layer : map->getLayers()) {
      drawLayer(layer, target, now, deferred_tiles);

      if (layer.getName() == "characters") {
        deferred_tiles.push_back(*character);

        std::stable_sort(deferred_tiles.begin(), deferred_tiles.end(), [&](auto a, auto b) {
          const auto z_index = [](const auto& o) { return *o.zIndex(); };
          return std::visit(z_index, a) < std::visit(z_index, b);
        });

        for (const auto object: deferred_tiles) {
          std::visit([&](const auto& o) { target.draw(o); }, object);
        }
      }
    }
  }

  void drawLayer(tson::Layer& layer, sf::RenderTarget& target, std::chrono::milliseconds now, std::vector<std::variant<Tile, Character>>& deferred_tiles) const {
    switch (layer.getType()) {

    case tson::LayerType::TileLayer:
      drawTileLayer(layer, target, now, deferred_tiles);
      break;
    case tson::LayerType::ObjectGroup:
      drawObjectLayer(layer, target);
      break;
    case tson::LayerType::ImageLayer:
      drawImageLayer(layer, target);
      break;
    case tson::LayerType::Group:
      for (auto& l : layer.getLayers()) {
        drawLayer(l, target, now, deferred_tiles);
      }
      break;
    default:
      break;
    }
  }

  mutable std::map<int, Animation> running_animations;

  void drawTileLayer(tson::Layer& layer, sf::RenderTarget& target, std::chrono::milliseconds now, std::vector<std::variant<Tile, Character>>& deferred_tiles) const {
    const auto player_texture_rect = character->getTextureBoundingRect();
    const auto player_texture_tile_from_x = static_cast<int>(player_texture_rect.left / getTileSize().x);
    const auto player_texture_tile_from_y = static_cast<int>(player_texture_rect.top / getTileSize().y);
    const auto player_texture_tile_to_x = static_cast<int>((player_texture_rect.left + player_texture_rect.width) / getTileSize().x);
    const auto player_texture_tile_to_y = static_cast<int>((player_texture_rect.top + player_texture_rect.height) / getTileSize().y);

    for (size_t x = from_x; x < to_x; x++) {
      for (size_t y = from_y; y < to_y; y++) {
        const auto* tileObjectP = layer.getTileObject(x, y);

        if (!tileObjectP) {
          continue;
        }

        auto tile = Tile(&layer, tileObjectP, asset_cache);
        tile.setScale(getScale());

        if (x >= player_texture_tile_from_x && x <= player_texture_tile_to_x &&
            y >= player_texture_tile_from_y && y <= player_texture_tile_to_y &&
            tile.zIndex()) {
          deferred_tiles.push_back(std::move(tile));
          continue;
        }

        target.draw(tile);
      }
    }
  }

  void drawImageLayer(tson::Layer& layer, sf::RenderTarget& target) const {
    auto texture = asset_cache->loadTexture(layer.getImage());
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

          auto texture = asset_cache->loadTexture(tileset->getImagePath());
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
        case tson::ObjectType::Rectangle:
          break;
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

      const size_t tileModX = (baseTilePosition % columns);
      const size_t currentRow = (baseTilePosition / columns);
      const float offsetX = (tileModX != 0) ? ((tileModX) * map->getTileSize().x) : (0 * map->getTileSize().x);
      const float offsetY = (currentRow < rows - 1) ? (currentRow * map->getTileSize().y) : ((rows - 1) * map->getTileSize().y);
      return sf::Vector2f(offsetX, offsetY);
    }

    return { 0.f, 0.f };
  }

  void createTileData(tson::Layer& layer) {
    if (layer.getType() == tson::LayerType::Group) {
      for (auto& nested: layer.getLayers()) {
        createTileData(nested);
      }
    } else if (layer.getType() == tson::LayerType::TileLayer) {
   		layer.assignTileMap((std::map<uint32_t, tson::Tile*>*)(&map->getTileMap()));
   		layer.createTileData(map->getSize(), map->isInfinite());
    }
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

  TiledMap(const fs::path& map_path, const std::shared_ptr<AssetCache> asset_cache_): asset_cache(asset_cache_) {
    dir = map_path.parent_path();
    filename = map_path.filename();

    tson::Tileson parser;
    map = parser.parse(map_path);

    if (map->getStatus() != tson::ParseStatus::OK) {
      throw std::runtime_error("Failed parsing '" + map_path.string() + "'.\n" + map->getStatusMessage());
    }

    for (auto& layer: map->getLayers()) {
      if (layer.getType() == tson::LayerType::Group) {
        createTileData(layer);
      }
    }

    for (const auto& tileset: map->getTilesets()) {
      asset_cache->loadTexture(tileset.getImagePath());
    }

    from_x = 0;
    to_x = map->getSize().x;
    from_y = 0;
    to_y = map->getSize().y;

    fixIndices();
  }

  void fixIndices() const {
    using json = nlohmann::json;
    const auto absolute_path = dir / filename;
    std::ifstream ifs(absolute_path);
    json jf = json::parse(ifs);

    for (const auto& sets : jf["tilesets"].items()) {
      const auto first_gid = sets.value()["firstgid"].get<int>();

      for (const auto& tile : sets.value()["tiles"].items()) {
        const auto id = first_gid - 1 + tile.value()["id"].get<int>();

        for (const auto& animation : tile.value()["animation"].items()) {
          const auto& tile_id = first_gid + animation.value()["tileid"].get<int>();
        }
      }
    }
  }

  sf::Vector2i mapCoordsToTile(const sf::Vector2f& coords) {
    const auto factor_x = getScale().x * getTileSize().x;
    const auto factor_y = getScale().y * getTileSize().y;

    return { static_cast<int>(coords.x / factor_x), static_cast<int>(coords.y / factor_y) };
  }

  void update(const sf::View& view, const sf::RenderWindow& window, const std::chrono::milliseconds& now) {
    const auto window_size = window.getSize();

    auto from = mapCoordsToTile(window.mapPixelToCoords({0, 0}, view));
    auto to = mapCoordsToTile(window.mapPixelToCoords({ static_cast<int>(window_size.x), static_cast<int>(window_size.y) }, view));

    // Update culling range.
    from_x = std::max(0, from.x);
    to_x = std::max(0, to.x) + 1;
    from_y = std::max(0, from.y);
    to_y = std::max(0, to.y) + 1;
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

  sf::Vector2f getView(const float window_width, const float window_height) {
    auto scale = getScale();
    auto character_position = character->getPosition();

    sf::Vector2f view({ character_position.x * scale.x, character_position.y * scale.y });

    if (view.x - window_width / 2.f < 0) {
      view.x = window_width / 2.f;
    }

    if (view.x + window_width / 2.f > getSize().x * scale.x) {
      view.x = getSize().x * scale.x - window_width / 2.f;
    }

    if (view.y - window_height / 2.f < 0) {
      view.y = window_height / 2.f;
    }

    if (view.y + window_height / 2.f > getSize().y * scale.y) {
      view.y = getSize().y * scale.y - window_height / 2.f;
    }

    return view;
  }

  void addCharacter(const Character* character) {
    this->character = character;
  }

  std::vector<sf::RectangleShape> collisionTiles(const Character& player) const {
    std::vector<sf::RectangleShape> shapes;

    const auto player_pos = player.getPosition();

    const auto player_tile_x = static_cast<int>(player_pos.x / getTileSize().x);
    const auto player_tile_y = static_cast<int>(player_pos.y / getTileSize().y);

    const auto [max_x, max_y] = map->getSize();

    auto create_collision_shape = [&player, &shapes] (const sf::FloatRect& rect) {
      sf::RectangleShape shape({ rect.width, rect.height });
      shape.setFillColor(sf::Color::Transparent);
      shape.setOutlineColor(sf::Color::Red);

      shape.setOutlineThickness(0.5f);
      shape.setPosition({ rect.left, rect.top });
      shapes.push_back(shape);
    };

    std::function<void(tson::Layer&)> create_collision_shapes;
    create_collision_shapes = [&, max_x = max_x, max_y = max_y](tson::Layer& layer){
      switch (layer.getType()) {
        case tson::LayerType::TileLayer:
          for (size_t x = std::max(0, player_tile_x - 1); x < std::min(max_x, player_tile_x + 2); x++) {
            for (size_t y = std::max(0, player_tile_y - 1); y < std::min(max_y, player_tile_y + 2); y++) {
              const auto* tileObjectP = layer.getTileObject(x, y);

              if (!tileObjectP) {
                continue;
              }

              const auto tile = Tile(&layer, tileObjectP, asset_cache);

              for (auto& collision_rect: tile.getCollisionRects()) {
                create_collision_shape(collision_rect);
              }
            }
          }
          break;
        case tson::LayerType::ObjectGroup:
          for (auto& obj : layer.getObjects()) {
            if (obj.getObjectType() == tson::ObjectType::Rectangle && obj.getType() == "collision") {
              sf::FloatRect object_rect = {
                static_cast<float>(obj.getPosition().x),
                static_cast<float>(obj.getPosition().y),
                static_cast<float>(obj.getSize().x),
                static_cast<float>(obj.getSize().y),
              };
              create_collision_shape(object_rect);
            }
          }
          break;
        case tson::LayerType::Group:
          for (auto& layer: layer.getLayers()) {
            create_collision_shapes(layer);
          }
          break;
        default:
          break;
      }
    };

    for (auto& layer: map->getLayers()) {
      create_collision_shapes(layer);
    }

    return shapes;
  }
};
