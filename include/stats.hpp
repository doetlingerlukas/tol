#pragma once

#include <thread>
#include <mutex>
#include <future>

class Stats {
private:
  size_t health = 100;
  std::mutex health_mutex;
  std::promise<void> exit_signal;
  std::future<void> future_obj;
  std::thread regen_thread;

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

  Stats() : future_obj(exit_signal.get_future()), regen_thread([this] (std::future<void> future) {
    while(future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
      for(int i = 0; i < 14; i++) {
        if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout)
          break;

          std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }

      if (health < 100) {
        std::lock_guard<std::mutex> guard(health_mutex);
        health += 2;
      }
    }
  }, std::move(future_obj)) { }

  ~Stats() {
    exit_signal.set_value();
    regen_thread.join();
  }
};

