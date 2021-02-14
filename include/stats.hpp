#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <iostream>
#include <map>

template<typename T>
class StatsProps {
  virtual void increase(T value) = 0;
  virtual T get() const = 0;
  virtual std::ostream& print(std::ostream& print) const = 0;

  friend std::ostream& operator<< (std::ostream& stream, const StatsProps& stats) {
    return stats.print(stream);
  }
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

  virtual void increase(size_t value) override {
    health = std::clamp<size_t>(0, 100, health + value);
  }

  virtual size_t get() const override {
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

  virtual std::ostream& print(std::ostream& out) const override {
    out << "health: " << health << "\n";
    return out;
  }
};


class Strength : public StatsProps<size_t> {
  int strength = 10;

public:
  void increase(size_t value) override {
    strength += value;
  }

  virtual size_t get() const override {
    return strength;
  }

  virtual std::ostream& print(std::ostream& out) const override {
    out << "strength: " << strength << "\n";
    return out;
  }
};

class Speed : public StatsProps<size_t> {
  int speed = 10;

public:
  void increase(size_t value) override {
    speed += value;
  }

  virtual size_t get() const override {
    return speed;
  }

  virtual std::ostream& print(std::ostream& out) const override {
    out << "speed: " << speed << "\n";
    return out;
  }
};

class Experience : public StatsProps<size_t> {
  size_t experience = 0;
  size_t level = 1;

  std::map<size_t, size_t> xp_bracket {
    {0, 1}, {100, 2}, {280, 3},
    {500, 4}, {870, 5}, {1300, 6},
    {2000, 7}, {3000, 8}, {4500, 9},
    {6600, 10}
  };

public:
  void increase(size_t value) override {
    experience += value;

    for(auto& [xp, lvl]: xp_bracket) {
      if(xp > experience) {
        break;
      }

      level = lvl;
    }
  }

  size_t get() const override {
    return experience;
  }

  virtual size_t getLevel() const {
    return level;
  }

  virtual std::ostream& print(std::ostream& out) const override {
    out << "experience: " << experience << ", level: " << level << "\n";
    return out;
  }
};

class Stats {
private:
  Health _health = Health();
  Strength _strength = Strength();
  Speed _speed = Speed();
  Experience _experience = Experience();

public:
  Health& health() {
    return _health;
  }

  Strength& strength() {
    return _strength;
  }

  Speed& speed() {
    return _speed;
  }

  Experience& experience() {
    return _experience;
  }

  void get() {
    std::cout << _health;
    std::cout << _strength;
    std::cout << _speed;
    std::cout << _experience;
  }

  Stats() { }
};

