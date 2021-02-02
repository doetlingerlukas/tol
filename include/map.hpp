#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

#include <animation.hpp>
#include <character.hpp>

class TiledMap: public sf::Drawable, public sf::Transformable {
  fs::path dir;
  fs::path filename;

  std::unique_ptr<tson::Map> map;

  size_t from_x;
  size_t to_x;
  size_t from_y;
  size_t to_y;

  mutable std::map<std::string, std::shared_ptr<const sf::Texture>> textures;
  const Character* character;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    std::vector<std::pair<tson::Layer*, const tson::TileObject*>> conflicting_tiles;

    for (auto& layer : map->getLayers()) {
      if (layer.getName() == "characters") {
        break;
      }

      drawLayer(layer, target, now, conflicting_tiles);
    }

    auto get_y = [this](auto pair) {
      const auto [_, tileObjectP] = pair;
      const auto& tileObject = *tileObjectP;
      auto& tile = *tileObject.getTile();

      const auto& tile_position = tileObject.getPosition();

      const auto y = tile.getProp("y")->template getValue<int>();
      const auto y_pos = tile_position.y + (y + 1) * this->getTileSize().y;

      return y_pos;
    };

    std::stable_sort(conflicting_tiles.begin(), conflicting_tiles.end(), [get_y](auto a, auto b) {
      return get_y(a) < get_y(b);
    });

    const float char_y = character->getTextureBoundingRect().top + character->getTextureBoundingRect().height;

    // TODO:
    //   - Sort character together with tiles.
    bool character_drawn = false;
    for (const auto pair: conflicting_tiles) {
      const auto y = get_y(pair);

      if (char_y <= y && !character_drawn) {
        target.draw(*character);
        character_drawn = true;
      }

      const auto [layer, tileObjectP] = pair;
      drawTile(*layer, target, now, *tileObjectP);
    }

    if (!character_drawn) {
      target.draw(*character);
    }

    bool character_layer_found = false;
    for (auto& layer : map->getLayers()) {
      if (layer.getName() == "characters") {
        character_layer_found = true;
        continue;
      } else if (!character_layer_found) {
        continue;
      }

      drawLayer(layer, target, now, conflicting_tiles);
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

  void drawLayer(tson::Layer& layer, sf::RenderTarget& target, std::chrono::milliseconds now, std::vector<std::pair<tson::Layer*, const tson::TileObject*>>& conflicting_tiles) const {
    switch (layer.getType()) {

    case tson::LayerType::TileLayer:
      drawTileLayer(layer, target, now, conflicting_tiles);
      break;
    case tson::LayerType::ObjectGroup:
      drawObjectLayer(layer, target);
      break;
    case tson::LayerType::ImageLayer:
      drawImageLayer(layer, target);
      break;
    case tson::LayerType::Group:
      for (auto& l : layer.getLayers()) {
        drawLayer(l, target, now, conflicting_tiles);
      }
      break;
    default:
      break;
    }
  }

  mutable std::map<int, Animation> running_animations;

  void drawCollisionRect(const sf::Rect<float>& rect, sf::RenderTarget& target) const {
    sf::Color collision_color(255, 0, 0, 100);

    const auto scale = getScale();
    sf::RectangleShape collision_box({ rect.width, rect.height });
    collision_box.setFillColor(collision_color);
    collision_box.setPosition({ rect.left * scale.x, rect.top * scale.y });
    collision_box.setScale(scale);

    target.draw(collision_box);
  }

  void drawTile(tson::Layer& layer, sf::RenderTarget& target, std::chrono::milliseconds now, const tson::TileObject& tileObject) const {
    const auto& tile = *tileObject.getTile();
    auto* tileset = tile.getTileset();

    tson::Rect tsonRect = tile.getDrawingRect();
    sf::IntRect rect = { tsonRect.x, tsonRect.y, tsonRect.width, tsonRect.height };

    const auto& animation = tile.getAnimation();

    if (animation.size() > 0) {
      const auto tile_id = tile.getGid();

      if (running_animations.count(tile_id) == 0) {
        std::cout << "Adding animation for tile " << tile_id << std::endl;
        running_animations.emplace(std::piecewise_construct, std::make_tuple(tile_id), std::make_tuple(animation, tileset));
      } else {
        rect = running_animations.at(tile_id).getDrawingRect(now);
      }
    }

    sf::Vector2f scale = getScale();

    sf::Vector2f origin = { rect.width / 2.f, rect.height / 2.f };
    const auto& tile_position = tileObject.getPosition();
    sf::Vector2f position = {
      (origin.x + tile_position.x + getPosition().x) * scale.x,
      (origin.y + tile_position.y + getPosition().y) * scale.y,
    };

    float rotation = 0.f;
    if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Diagonally))
      rotation += 90.f;

    if (layer.getName() != "collision") {
      auto texture = loadImage(tileset->getImagePath());
      sf::Sprite sprite(*texture, rect);

      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Horizontally))
        scale.x = -scale.x;
      if (tileObject.getTile()->hasFlipFlags(tson::TileFlipFlags::Vertically))
        scale.y = -scale.y;

      sprite.setOrigin(origin);
      sprite.setPosition(position);
      sprite.setScale(scale);
      sprite.setRotation(rotation);

      target.draw(sprite);
    }

    // Draw collision boxes.
    auto object_group = tile.getObjectgroup();
    for (auto& object: object_group.getObjects()) {
      drawCollisionRect(
        {
          (tile_position.x + getPosition().x + object.getPosition().x),
          (tile_position.y + getPosition().y + object.getPosition().y),
          static_cast<float>(object.getSize().x),
          static_cast<float>(object.getSize().y),
        },
        target
      );
    }
  }

  void drawTileLayer(tson::Layer& layer, sf::RenderTarget& target, std::chrono::milliseconds now, std::vector<std::pair<tson::Layer*, const tson::TileObject*>>& conflicting_tiles) const {
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

        const auto& tileObject = *tileObjectP;

        if (x >= player_texture_tile_from_x && x <= player_texture_tile_to_x && y >= player_texture_tile_from_y && y <= player_texture_tile_to_y) {
          auto& tile = *tileObject.getTile();
          const auto* y = tile.getProp("y");

          if (y) {
            std::pair<tson::Layer*, const tson::TileObject*> item = std::make_pair(&layer, tileObjectP);
            conflicting_tiles.push_back(item);
            continue;
          }
        }

        drawTile(layer, target, now, tileObject);
      }
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
        case tson::ObjectType::Rectangle:
          drawCollisionRect(
            {
              static_cast<float>(obj.getPosition().x),
              static_cast<float>(obj.getPosition().y),
              static_cast<float>(obj.getSize().x),
              static_cast<float>(obj.getSize().y),
            },
            target
          );
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

  void update(const sf::View& view, const sf::RenderWindow& window) {
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
      shape.setOutlineColor(sf::Color::Blue);

      if (player.getBoundingRect().intersects(rect)) {
        shape.setOutlineColor(sf::Color::Red);
      }

      shape.setOutlineThickness(0.5f);
      shape.setPosition({ rect.left, rect.top });
      shapes.push_back(shape);
    };

    for (auto& layer: map->getLayers()) {
      for (size_t x = std::max(0, player_tile_x - 1); x < std::min(max_x, player_tile_x + 2); x++) {
        for (size_t y = std::max(0, player_tile_y - 1); y < std::min(max_y, player_tile_y + 2); y++) {
          const auto* tileObjectP = layer.getTileObject(x, y);

          if (!tileObjectP) {
            continue;
          }

          const auto& tileObject = *tileObjectP;

          const auto& tile_position = tileObject.getPosition();
          sf::FloatRect tile_rect = {
            tile_position.x + getPosition().x,
            tile_position.y + getPosition().y,
            static_cast<float>(getTileSize().x),
            static_cast<float>(getTileSize().y),
          };

          auto object_group = (*tileObject.getTile()).getObjectgroup();
          for (auto& object: object_group.getObjects()) {
            const auto [object_x, object_y] = object.getPosition();
            sf::FloatRect object_rect = {
              tile_rect.left + object_x,
              tile_rect.top + object_y,
              static_cast<float>(object.getSize().x),
              static_cast<float>(object.getSize().y),
            };
            create_collision_shape(object_rect);
          }
        }
      }

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
    }


    return shapes;
  }
};
