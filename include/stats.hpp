#pragma once

class Stats {
private:
  float health = 100;

  float REGENERATION_SPEED = 1;

public:
  float getHealth() {
    return health;
  }

  void setHealth(float health) {
    this->health = std::clamp(health, 0.f, 100.f);
  }

  void decrementHealth(float value) {
    setHealth(health - value);
  }

  void regenerateHealth(float dt) {
    setHealth(health + REGENERATION_SPEED * dt);
  }
};
