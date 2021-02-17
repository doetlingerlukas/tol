#include <tile.hpp>

std::map<int, Animation> Tile::running_animations;

void Tile::draw(sf::RenderTarget& target, sf::RenderStates state) const {
  auto& tileset = *getTile().getTileset();

  tson::Rect tsonRect = getTile().getDrawingRect();
  sf::IntRect rect = { tsonRect.x, tsonRect.y, tsonRect.width, tsonRect.height };

  const auto& animation = getTile().getAnimation();

  if (animation.size() > 0) {
    const auto tile_id = getTile().getGid();

    if (running_animations.count(tile_id) == 0) {
      std::cout << "Adding animation for tile " << tile_id << std::endl;
      running_animations.emplace(
        std::piecewise_construct, std::make_tuple(tile_id), std::make_tuple(animation, std::ref(tileset)));
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
  if (getTile().hasFlipFlags(tson::TileFlipFlags::Diagonally))
    rotation += 90.f;

  auto texture = asset_cache->loadTexture(tileset.getImagePath());
  sf::Sprite sprite(*texture, rect);

  if (getTile().hasFlipFlags(tson::TileFlipFlags::Horizontally))
    scale.x = -scale.x;
  if (getTile().hasFlipFlags(tson::TileFlipFlags::Vertically))
    scale.y = -scale.y;

  sprite.setOrigin(origin);
  sprite.setPosition(position);
  sprite.setScale(scale);
  sprite.setRotation(rotation);

  target.draw(sprite);
}

Tile::Tile(tson::Tile& tile_, const sf::Vector2f& position, std::shared_ptr<AssetCache> asset_cache_):
  asset_cache(asset_cache_), tile(tile_) {
  setPosition({ position.x, position.y });

  const auto y_prop = getTile().getProp("y");
  if (y_prop) {
    z_index = position.y + (y_prop->template getValue<int>() + 1) * getTile().getTileSize().y;
  }
}

Tile::Tile(const tson::TileObject& object, std::shared_ptr<AssetCache> asset_cache):
  Tile(*object.getTile(), { object.getPosition().x, object.getPosition().y }, asset_cache) {}

std::optional<float> Tile::zIndex() const {
  return z_index;
}

sf::FloatRect Tile::getBoundingRect() const {
  tson::Rect tsonRect = getTile().getDrawingRect();
  return { getPosition().x, getPosition().y, static_cast<float>(tsonRect.width), static_cast<float>(tsonRect.height) };
}

std::vector<sf::FloatRect> Tile::getCollisionRects() const {
  std::vector<sf::FloatRect> collision_rects;

  auto object_group = getTile().getObjectgroup();
  for (auto& object: object_group.getObjects()) {
    collision_rects.emplace_back(
      (getPosition().x + object.getPosition().x), (getPosition().y + object.getPosition().y),
      static_cast<float>(object.getSize().x), static_cast<float>(object.getSize().y));
  }

  return collision_rects;
}

void Tile::update(std::chrono::milliseconds now) {
  this->now = now;
}
