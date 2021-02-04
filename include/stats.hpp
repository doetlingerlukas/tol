#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <iostream>

class StatsEvents {
  virtual void subscribe(std::function<void()> func) = 0;
};

class Health : public StatsEvents {
private:
  size_t health = 100;
  std::mutex health_mutex;
  std::promise<void> exit_signal;
  std::future<void> future_obj;
  std::thread regen_thread;
  std::function<void()> callback;

public:
  void subscribe(std::function<void()> func) override {
    callback = func;
  }

  size_t get() {
    return health;
  }

  void decrement(size_t value) {
    if (health == 0 || health == value) {
      health = 0;
      callback();
      return;
    }

    health = health - value;
  }

  Health(const Health &h) { }

  Health() : future_obj(exit_signal.get_future()), regen_thread([this] (std::future<void> future) {
    while(future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
      bool damage_received = false;

      while(health == 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      auto health_before = health;
      for(int i = 0; i < 14; i++) {
        if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout || health_before != health) {
          damage_received = true;
          break;
        } else {
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
      }

      if (!damage_received && health < 100) {
        std::lock_guard<std::mutex> guard(health_mutex);
        health += 2;
      }
    }
  }, std::move(future_obj)) { }

  ~Health() {
    exit_signal.set_value();
    regen_thread.join();
  }
};

class Stats {
private:
  Health _health = Health();

public:
  Health& health() {
    return _health;
  }

  Stats() { }
};

