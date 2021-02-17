#pragma once

class Attack {
  std::string name;
  const int damage;

  public:
  Attack(const std::string& name_, const int damage_): name(name_), damage(damage_) {}

  [[nodiscard]] std::string getName() const {
    return name;
  }

  [[nodiscard]] int getDamage() const {
    return damage;
  }
};
