#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <iostream>

template<typename T>
class StatsProps {
  virtual void increase(T value) = 0;
  virtual T get() = 0;
};

class Health : public StatsProps<size_t> {
private:
  size_t health = 100;
  std::mutex health_mutex;
  std::promise<void> exit_signal;
  std::future<void> future_obj;
  std::thread regen_thread;
  std::function<void()> callback;

public:
  void subscribe(std::function<void()> func) {
    callback = func;
  }

  void increase(size_t value) override {
    health = std::clamp<size_t>(0, 100, health + value);
  }

  size_t get() override {
    return health;
  }

  void decrease(size_t value) {
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

      while(health == 100 && future.wait_for(std::chrono::milliseconds(1)) != std::future_status::timeout) {
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

class Strength : public StatsProps<size_t> {
  int _strength = 10;

  void increase(size_t value) override { }

  size_t get() override {
    return _strength;
  }
};

class Speed : public StatsProps<size_t> {
  int _speed = 10;

  void increase(size_t value) override { }

  size_t get() override {
    return _speed;
  }
};

class Stats {
private:
  Health _health = Health();
  Strength _strength = Strength();
  Speed _speed = Speed();

public:
  Health& health() {
    return _health;
  }

  Strength& strength() {
    return _strength;
  }

  Speed speed() {
    return _speed;
  }

  Stats() { }
};

