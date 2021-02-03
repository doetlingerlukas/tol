#pragma once

#include <thread>
#include <mutex>

class Stats {
private:
  size_t health = 100;
  std::mutex healthMutex;
  std::thread regenThread;

public:
  size_t getHealth() {
    return health;
  }

  void decrementHealth(size_t value) {
    if (health == 0 || health == value) {
      health = 0;
      return;
    }

    health = health - value;
  }

  Stats() : regenThread([this] {
    while(true) {
      std::this_thread::sleep_for(std::chrono::seconds(7));

      if (health < 100) {
        std::lock_guard<std::mutex> guard(healthMutex);
        health += 2;
      }
    }
  }) { }

  ~Stats() { regenThread.join(); }
};

