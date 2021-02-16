#pragma once

#include <string>
#include <map>


class Collectible {
public:
  const std::string info;

  explicit Collectible(const std::string& info_) : info(info_) {}
};

std::map<std::string, Collectible> getCollectibles() {
  std::map<std::string, Collectible> collectibles;

  collectibles.insert({ "lemon", Collectible("A sour fruit.") });
  collectibles.insert({ "strawberry", Collectible("Very sweet and delicious.") });
  collectibles.insert({ "orange", Collectible("Makes a perfect juice.") });
  collectibles.insert({ "melon", Collectible("A watermelon.") });
  collectibles.insert({ "pear", Collectible("Increases your speed.") });

  return collectibles;
}

Collectible getCollectible(const std::string name) {
  auto collectibles = getCollectibles();
  auto search = collectibles.find(name);
  if (search == collectibles.end()) {
    return Collectible("Undefined object.");
  }
  return search->second;
}