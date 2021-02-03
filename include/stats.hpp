#pragma once

class Stats {
private:
  size_t health = 100;

public:
  size_t getHealth() {
    return health;
  }

  void decrementHealth(size_t value) {
    if ((health - value) <= 0) {
      health = 0;
      return;
    }

    health = health - value;

  }

  Stats() { }
};

