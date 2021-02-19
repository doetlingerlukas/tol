#include "tile.hpp"

namespace tol {

std::map<int, Animation> Tile::running_animations;

void Tile::draw(sf::RenderTarget& target, sf::RenderStates state) const {
  auto& tileset = *tile().getTileset();

  tson::Rect tson_rect = tile().getDrawingRect();
  sf::IntRect rect = { tson_rect.x, tson_rect.y, tson_rect.width, tson_rect.height };

  const auto& animation = tile().getAnimation();

  if (animation.size() > 0) {
    const auto tile_id = tile().getGid();

    if (running_animations.count(tile_id) == 0) {
      std::cout << "Adding animation for tile " << tile_id << std::endl;
      running_animations.emplace(
        std::piecewise_construct, std::make_tuple(tile_id), std::make_tuple(animation, std::ref(tileset)));
    } else {
      rect = running_animations.at(tile_id).drawing_rect(now);
    }
  }

  sf::Vector2f scale = getScale();

  sf::Vector2f origin = { rect.width / 2.f, rect.height / 2.f };
  sf::Vector2f position = {
    (origin.x + getPosition().x) * scale.x,
    (origin.y + getPosition().y) * scale.y,
  };

  float rotation = 0.f;
  if (tile().hasFlipFlags(tson::TileFlipFlags::Diagonally))
    rotation += 90.f;

  auto texture = asset_cache->load_texture(tileset.getImagePath());
  sf::Sprite sprite(*texture, rect);

  if (tile().hasFlipFlags(tson::TileFlipFlags::Horizontally))
    scale.x = -scale.x;
  if (tile().hasFlipFlags(tson::TileFlipFlags::Vertically))
    scale.y = -scale.y;

  sprite.setOrigin(origin);
  sprite.setPosition(position);
  sprite.setScale(scale);
  sprite.setRotation(rotation);

  target.draw(sprite);
}

Tile::Tile(tson::Tile& tile, const sf::Vector2f& position, std::shared_ptr<AssetCache> asset_cache_):
  asset_cache(asset_cache_), _tile(tile) {
  setPosition({ position.x, position.y });

  const auto y_prop = tile.getProp("y");
  if (y_prop) {
    _z_index = position.y + (y_prop->template getValue<int>() + 1) * tile.getTileSize().y;
  }
}

Tile::Tile(const tson::TileObject& object, std::shared_ptr<AssetCache> asset_cache):
  Tile(*object.getTile(), { object.getPosition().x, object.getPosition().y }, asset_cache) {}

std::optional<float> Tile::z_index() const {
  return _z_index;
}

sf::FloatRect Tile::bounds() const {
  tson::Rect tson_rect = tile().getDrawingRect();
  return { getPosition().x, getPosition().y, static_cast<float>(tson_rect.width),
           static_cast<float>(tson_rect.height) };
}

std::vector<sf::FloatRect> Tile::collisions() const {
  std::vector<sf::FloatRect> collision_rects;

  auto object_group = tile().getObjectgroup();
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

} // namespace tol
