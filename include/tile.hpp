#pragma once

#include <SFML/Graphics.hpp>

#include <nlohmann/json.hpp>
#include <tileson.hpp>

#include <asset_cache.hpp>
#include <animation.hpp>
#include <z_indexable.hpp>

class Tile: public ZIndexable, public sf::Drawable, public sf::Transformable {
  std::shared_ptr<AssetCache> asset_cache;

  static std::map<int, Animation> running_animations;

  tson::Tile* tile;

  std::optional<float> z_index;

  std::chrono::milliseconds now;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates state) const {
    auto* tileset = tile->getTileset();

    tson::Rect tsonRect = tile->getDrawingRect();
    sf::IntRect rect = { tsonRect.x, tsonRect.y, tsonRect.width, tsonRect.height };

    const auto& animation = tile->getAnimation();

    if (animation.size() > 0) {
      const auto tile_id = tile->getGid();

      if (running_animations.count(tile_id) == 0) {
        std::cout << "Adding animation for tile " << tile_id << std::endl;
        running_animations.emplace(std::piecewise_construct, std::make_tuple(tile_id), std::make_tuple(animation, tileset));
      } else {
        rect = running_animations.at(tile_id).getDrawingRect(now);
      }
    }

    sf::Vector2f scale = getScale();

    sf::Vector2f origin = { rect.width / 2.f, rect.height / 2.f };
    sf::Vector2f position = {
      (origin.x + getPosition().x) * scale.x,
      (origin.y + getPosition().y) * scale.y,
    };

    float rotation = 0.f;
    if (tile->hasFlipFlags(tson::TileFlipFlags::Diagonally))
      rotation += 90.f;

    auto texture = asset_cache->loadTexture(tileset->getImagePath());
    sf::Sprite sprite(*texture, rect);

    if (tile->hasFlipFlags(tson::TileFlipFlags::Horizontally))
      scale.x = -scale.x;
    if (tile->hasFlipFlags(tson::TileFlipFlags::Vertically))
      scale.y = -scale.y;

    sprite.setOrigin(origin);
    sprite.setPosition(position);
    sprite.setScale(scale);
    sprite.setRotation(rotation);

    target.draw(sprite);
  }

protected:
  tson::Tile* getTile() const {
    return tile;
  }

public:
  Tile(tson::Tile* tile_, const sf::Vector2f& position, std::shared_ptr<AssetCache> asset_cache_): tile(tile_), asset_cache(asset_cache_) {
    setPosition({ position.x, position.y });

    const auto y_prop = tile->getProp("y");
    if (y_prop) {
      z_index = position.y + (y_prop->template getValue<int>() + 1) * tile->getTileSize().y;
    }
  }

  Tile(const tson::TileObject* object, std::shared_ptr<AssetCache> asset_cache):
    Tile(object->getTile(), { object->getPosition().x, object->getPosition().y }, asset_cache) {}

  virtual std::optional<float> zIndex() const {
    return z_index;
  }

  std::vector<sf::FloatRect> getCollisionRects() const {
    std::vector<sf::FloatRect> collision_rects;

    auto object_group = tile->getObjectgroup();
    for (auto& object: object_group.getObjects()) {
      collision_rects.emplace_back(
        (getPosition().x + object.getPosition().x),
        (getPosition().y + object.getPosition().y),
        static_cast<float>(object.getSize().x),
        static_cast<float>(object.getSize().y)
      );
    }

    return collision_rects;
  }

  void update(std::chrono::milliseconds now) {
    this->now = now;
  }
};

std::map<int, Animation> Tile::running_animations;
