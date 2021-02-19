#pragma once

#include <map>
#include <string>

namespace tol {

class Collectible {
  const int id_;
  const std::string name_;
  const std::string info_;

  inline static std::map<std::string, std::string> descriptions{
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

  [[nodiscard]] inline int id() const {
    return id_;
  }

  [[nodiscard]] inline const std::string& name() const {
    return name_;
  }

  [[nodiscard]] inline const std::string& info() const {
    return info_;
  }
};

} // namespace tol
