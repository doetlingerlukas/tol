#pragma once

#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>

using json = nlohmann::json;

template <typename T> class StatsProps {
  virtual void increase(T value) = 0;
  virtual T get() const = 0;
  virtual std::ostream& print(std::ostream& print) const = 0;

  friend std::ostream& operator<<(std::ostream& stream, const StatsProps& stats) {
    return stats.print(stream);
  }
};

class Health: public StatsProps<size_t> {
  private:
  size_t health = 100;
  std::function<void()> callback = nullptr;

  public:
  void subscribe(std::function<void()> func);

  virtual void increase(size_t value) override;

  virtual size_t get() const override;

  void decrease(size_t value);

  Health(const Health& h) = delete;

  Health(size_t health_);

  virtual std::ostream& print(std::ostream& out) const override;
};


class Strength : public StatsProps<size_t> {
  size_t strength = 10;

  public:
  Strength(size_t strength_);

  void increase(size_t value) override;

  virtual size_t get() const override;

  virtual std::ostream& print(std::ostream& out) const override;
};

class Speed : public StatsProps<size_t> {
  size_t speed = 10;

  public:
  Speed(size_t speed_);

  void increase(size_t value) override;

  virtual size_t get() const override;

  virtual std::ostream& print(std::ostream& out) const override;
};

class Experience: public StatsProps<size_t> {
  size_t experience = 0;
  size_t level = 1;

  std::map<size_t, size_t> xp_bracket{ { 0, 1 },    { 100, 2 },  { 280, 3 },  { 500, 4 },  { 870, 5 },
                                       { 1300, 6 }, { 2000, 7 }, { 3000, 8 }, { 4500, 9 }, { 6600, 10 } };

  public:
  Experience(size_t lvl);

  void increase(size_t value) override;

  size_t get() const override;

  virtual size_t getLevel() const;

  virtual std::ostream& print(std::ostream& out) const override;
};

class Stats {
  private:
  Health _health;
  Strength _strength;
  Speed _speed;
  Experience _experience;

  public:
  Stats(const json& stats);

  Health& health();

  Strength& strength();

  Speed& speed();

  Experience& experience();

  void get();
};
