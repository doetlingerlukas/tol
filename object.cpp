#include "object.hpp"

namespace tol {

const std::map<std::string, std::string> Object::DESCRIPTIONS{
  { "lemon", "A sour fruit. Can be used to gain 30 health." },
  { "strawberry", "Very sweet and delicious. Also adds 400 experience" },
  { "orange", "Makes a perfect juice. Increases strength by 1." },
  { "melon", "A tremendous watermelon. Eating it recovers 50 health." },
  { "pear", "Looks just like a regular fruit, but think twice before eating. Not everyone can handle the speed. +20 "
            "on walking speed" },
  { "cherry", "Probably edible." },
  { "pistol", "Watch out! This kills people." },
  { "tools", "I should probably bring these back to Detlef." },
  { "baguette", "Perfect for stabbing someone." }
};

const std::string Object::UNDEFINED_DESCRIPTION = "Undefined object.";

const std::map<std::string, std::function<std::optional<std::string>(Protagonist& player)>> Object::EFFECTS{
  { "lemon",
    [](auto& player) {
      player.stats().health().increase(30);
      return player.stats().get();
    } },
  { "strawberry",
    [](auto& player) {
      player.stats().experience().increase(400);
      return player.stats().get();
    } },
  { "orange",
    [](auto& player) {
      player.stats().strength().increase(1);
      return player.stats().get();
    } },
  { "melon",
    [](auto& player) {
      player.stats().health().increase(50);
      return player.stats().get();
    } },
  { "pear",
    [](auto& player) {
      player.stats().speed().increase(20);
      return player.stats().get();
    } },
  { "pistol",
    [](auto& player) {
      player.add_attack(Attack("pistol", 25));
      return "Pistol available for fight.";
    } },
  { "baguette",
    [](auto& player) {
      player.add_attack(Attack("baguette", 18));
      return "Baguette available for fight.";
    } }
};

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
  auto found = DESCRIPTIONS.find(name());
  if (found != DESCRIPTIONS.end()) {
    return found->second;
  } else {
    return UNDEFINED_DESCRIPTION;
  }
}

std::optional<std::function<std::optional<std::string>(Protagonist& player)>> Object::effect() const {
  auto found = Object::EFFECTS.find(name());
  if (found != Object::EFFECTS.end()) {
    return found->second;
  }

  return std::nullopt;
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
