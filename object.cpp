#include "object.hpp"

namespace tol {

Object::Object(tson::Object& object, tson::Tile& tile, std::shared_ptr<AssetCache> asset_cache):
  Tile(
    tile,
    // Y for tile objects is on bottom, so go up one tile.
    // See https://github.com/mapeditor/tiled/issues/91.
    { static_cast<float>(object.getPosition().x), static_cast<float>(object.getPosition().y - tile.getTileSize().y) },
    asset_cache),
  object(object) {}

bool Object::intersects(const sf::FloatRect& rect) const {
  return rect.intersects({ getPosition().x, getPosition().y, static_cast<float>(tile().getTileSize().y),
                           static_cast<float>(tile().getTileSize().y) });
}

bool Object::collides_with(const sf::FloatRect& rect) const {
  auto object_group = tile().getObjectgroup();
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

const std::string& Object::name() const {
  return object.get().getName();
}

const std::string& Object::description() const {
  auto search = DESCRIPTIONS.find(name());
  if (search != DESCRIPTIONS.end()) {
    return search->second;
  } else {
    return UNDEFINED_DESCRIPTION;
  }
}

std::optional<float> Object::z_index() const {
  auto prop = object.get().getProp("always_on_top");
  if (prop && std::any_cast<const bool&>(prop->getValue())) {
    return std::numeric_limits<float>::infinity();
  }

  return getPosition().y + tile().getTileSize().y;
}

bool Object::usable() const {
  auto prop = object.get().getProp("usable");
  if (prop) {
    return std::any_cast<const bool&>(prop->getValue());
  }

  return true;
}

} // namespace tol
