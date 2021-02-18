#pragma once

#include <map>
#include <string>

class Collectible {
  const int id_;
  const std::string name_;
  const std::string info_;

  inline static std::map<std::string, std::string> descriptions{
    { "lemon", "A sour fruit." }, { "strawberry", "Very sweet and delicious." }, { "orange", "Makes a perfect juice." },
    { "melon", "A watermelon." }, { "pear", "Increases your speed." },           { "cherry", "Probably edible." },
  };

  public:
  explicit Collectible(const std::string& name, const std::string& info): id_(0), name_(""), info_(info) {}

  static Collectible getCollectible(const std::string name) {
    auto search = descriptions.find(name);
    if (search != descriptions.end()) {
      return Collectible(name, search->second);
    } else {
      return Collectible(name, "Undefined object.");
    }
  }

  inline int id() const {
    return id_;
  }

  inline const std::string& name() const {
    return name_;
  }

  inline const std::string& info() const {
    return info_;
  }
};
