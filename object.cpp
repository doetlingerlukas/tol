#include <object.hpp>

Object::Object(const tson::Object& object, tson::Tile& tile, std::shared_ptr<AssetCache> asset_cache):
  Tile(
    tile,
    // Y for tile objects is on bottom, so go up one tile.
    // See https://github.com/mapeditor/tiled/issues/91.
    { static_cast<float>(object.getPosition().x), static_cast<float>(object.getPosition().y - tile.getTileSize().y) },
    asset_cache),
  object(object) {}

bool Object::intersects(const sf::FloatRect& rect) const {
  return rect.intersects({ getPosition().x, getPosition().y, static_cast<float>(getTile().getTileSize().y),
                           static_cast<float>(getTile().getTileSize().y) });
}

bool Object::collides_with(const sf::FloatRect& rect) const {
  auto object_group = getTile().getObjectgroup();
  for (auto& obj: object_group.getObjects()) {
    sf::FloatRect object_rect = {
      static_cast<float>(getPosition().x + obj.getPosition().x),
      static_cast<float>(getPosition().y + obj.getPosition().y),
      static_cast<float>(obj.getSize().x),
      static_cast<float>(obj.getSize().y),
    };

    if (object_rect.intersects(rect)) {
      return true;
    }
  }

  return false;
}

const std::string& Object::getName() const {
  return object.get().getName();
}

std::optional<float> Object::zIndex() const {
  return getPosition().y + getTile().getTileSize().y;
}
