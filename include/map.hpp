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
#include <object.hpp>
#include <animation.hpp>
#include <character.hpp>
#include <npc.hpp>
#include <protagonist.hpp>

class TiledMap: public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  fs::path dir;
  fs::path filename;

  std::unique_ptr<tson::Map> map;
  std::map<int, Object> collectibles;

  size_t from_x;
  size_t to_x;
  size_t from_y;
  size_t to_y;

  Protagonist* player;
  std::vector<Npc> npcs;
  std::vector<const Character*> characters;
  std::chrono::milliseconds now;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    std::vector<std::variant<Tile, Character, Object>> deferred_tiles;

    for (auto& layer : map->getLayers()) {
      drawLayer(layer, target, deferred_tiles);

      if (layer.getName() == "characters") {
        for (const auto* character: characters) {
          deferred_tiles.push_back(*character);
        }

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

  void drawLayer(tson::Layer& layer, sf::RenderTarget& target, std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const {
    if (layer.getName() == "collision") {
      return;
    }

    switch (layer.getType()) {
      case tson::LayerType::TileLayer:
        drawTileLayer(layer, target, deferred_tiles);
        break;
      case tson::LayerType::ObjectGroup:
        drawObjectLayer(layer, target, deferred_tiles);
        break;
      case tson::LayerType::ImageLayer:
        drawImageLayer(layer, target);
        break;
      case tson::LayerType::Group:
        for (auto& l : layer.getLayers()) {
          drawLayer(l, target, deferred_tiles);
        }
        break;
      default:
        break;
    }
  }

  mutable std::map<int, Animation> running_animations;

  void drawTileLayer(tson::Layer& layer, sf::RenderTarget& target, std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const {
    std::vector<std::pair<sf::Vector2i, sf::Vector2i>> character_rects;

    for (const auto* character: characters) {
      const auto texture_rect = character->getTextureBoundingRect();
      const auto texture_tile_from_x = static_cast<int>(texture_rect.left / getTileSize().x);
      const auto texture_tile_from_y = static_cast<int>(texture_rect.top / getTileSize().y);
      const auto texture_tile_to_x = static_cast<int>((texture_rect.left + texture_rect.width) / getTileSize().x);
      const auto texture_tile_to_y = static_cast<int>((texture_rect.top + texture_rect.height) / getTileSize().y);

      character_rects.emplace_back(sf::Vector2i{texture_tile_from_x, texture_tile_from_y}, sf::Vector2i{texture_tile_to_x, texture_tile_to_y});
    }

    for (size_t x = from_x; x < to_x; x++) {
      for (size_t y = from_y; y < to_y; y++) {
        const auto* tileObjectP = layer.getTileObject(x, y);

        if (!tileObjectP) {
          continue;
        }

        auto tile = Tile(*tileObjectP, asset_cache);
        tile.setScale(getScale());
        tile.update(now);

        if (tile.zIndex() && std::any_of(character_rects.begin(), character_rects.end(), [x, y](const auto& r) {
          const auto& [from, to] = r;
          return x >= from.x && x <= to.x &&
                 y >= from.y && y <= to.y;
         })) {
          deferred_tiles.emplace_back(std::move(tile));
        } else {
          target.draw(tile);
        }
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

  void drawObjectLayer(tson::Layer& layer, sf::RenderTarget& target, std::vector<std::variant<Tile, Character, Object>>& deferred_tiles) const {
    auto* map = layer.getMap();
    for (auto& obj : layer.getObjects()) {
      switch (obj.getObjectType()) {
        case tson::ObjectType::Object: {
          const auto id = obj.getId();
          if (collectibles.count(id) != 0 && collectibles.at(id).intersects(window_rect)) {
            auto object = collectibles.at(id);
            object.setScale(getScale());
            object.update(now);

            if (std::any_of(characters.begin(), characters.end(), [&object](const auto& c) {
              return object.intersects(c->getTextureBoundingRect());
            })) {
              deferred_tiles.emplace_back(std::move(object));
            } else {
              target.draw(object);
            }
          }

          break;
        }
        case tson::ObjectType::Rectangle:
          break;
        default:
          break;
      }
    }
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

  void gatherCollectibles(tson::Layer& layer) {
    if (layer.getType() == tson::LayerType::Group) {
      for (auto& nested: layer.getLayers()) {
        gatherCollectibles(nested);
      }
    } else if (layer.getType() == tson::LayerType::ObjectGroup) {
      for (auto& obj : layer.getObjects()) {
        if (obj.getObjectType() == tson::ObjectType::Object && obj.getType() == "collectible") {
          auto* tileset = map->getTilesetByGid(obj.getGid());

          collectibles.emplace(
            std::piecewise_construct,
            std::make_tuple(obj.getId()),
            std::make_tuple(std::cref(obj), std::ref(*tileset->getTile(obj.getGid())), asset_cache)
          );
        }
      }
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

      gatherCollectibles(layer);
    }

    for (const auto& tileset: map->getTilesets()) {
      asset_cache->loadTexture(tileset.getImagePath());
    }

    auto* character_layer = map->getLayer("characters");

    if (character_layer) {
      for (auto& character: character_layer->getObjects()) {
        const std::string& texture = std::any_cast<const std::string&>(character.getProp("texture")->getValue());
        fmt::print("Adding NPC with texture: {} and name: {}\n", texture, character.getName());
        Npc npc = Npc(texture, asset_cache, character.getName());
        npc.setPosition({ static_cast<float>(character.getPosition().x), static_cast<float>(character.getPosition().y) });
        npcs.push_back(std::move(npc));
      }
    }

    for (const auto& npc: npcs) {
      characters.push_back(&npc);
    }

    from_x = 0;
    to_x = map->getSize().x;
    from_y = 0;
    to_y = map->getSize().y;
  }

  sf::Vector2i mapCoordsToTile(const sf::Vector2f& coords) {
    const auto factor_x = getScale().x * getTileSize().x;
    const auto factor_y = getScale().y * getTileSize().y;

    return { static_cast<int>(coords.x / factor_x), static_cast<int>(coords.y / factor_y) };
  }

  sf::FloatRect getWindowRect() {
    return window_rect;
  }

  sf::FloatRect window_rect;

  void update(const sf::View& view, const sf::RenderWindow& window, const std::chrono::milliseconds& now) {
    this->now = now;

    const auto window_size = window.getSize();

    const auto from_coords = window.mapPixelToCoords({0, 0}, view);
    const auto from_tile = mapCoordsToTile(from_coords);
    const auto to_coords = window.mapPixelToCoords({ static_cast<int>(window_size.x), static_cast<int>(window_size.y) }, view);
    const auto to_tile = mapCoordsToTile(to_coords);

    window_rect = {
      static_cast<float>(from_coords.x) / getScale().x,
      static_cast<float>(from_coords.y) / getScale().y,
      static_cast<float>(to_coords.x - from_coords.x) / getScale().x,
      static_cast<float>(to_coords.y - from_coords.y) / getScale().y
    };

    // Update culling range.
    from_x = std::max(0, from_tile.x);
    to_x = std::max(0, to_tile.x) + 1;
    from_y = std::max(0, from_tile.y);
    to_y = std::max(0, to_tile.y) + 1;
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
    auto character_position = player->getPosition();

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

  void setPlayer(Protagonist* player) {
    this->player = player;
    addCharacter(player);
  }

  void addCharacter(const Character* character) {
    characters.push_back(character);
  }

  std::map<int, Object>& getCollectibles() {
    return collectibles;
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
          for (size_t x = std::max(0, player_tile_x - 2); x < std::min(max_x, player_tile_x + 3); x++) {
            for (size_t y = std::max(0, player_tile_y - 2); y < std::min(max_y, player_tile_y + 3); y++) {
              const auto* tileObjectP = layer.getTileObject(x, y);

              if (!tileObjectP) {
                continue;
              }

              auto tile = Tile(*tileObjectP, asset_cache);
              tile.setScale(getScale());
              tile.update(now);

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

  void setScale(float factorX, float factorY) {
    for (auto& npc: npcs) {
      npc.setScale(factorX, factorY);
    }

    sf::Transformable::setScale(factorX, factorY);
  }

  void setScale(const sf::Vector2f factors) {
    for (auto& npc: npcs) {
      npc.setScale(factors);
    }

    sf::Transformable::setScale(factors);
  }

  std::vector<Npc>& getNpcs() {
    return npcs;
  }
};
