#pragma once

class Stats {
private:
  size_t health = 40;

public:
  size_t getHealth() {
    return health;
  }

  void setHealth(size_t newHealth) {
    health = newHealth;
  }

  Stats() { }
};

