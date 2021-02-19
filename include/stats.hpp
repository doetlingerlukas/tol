#pragma once

#include <iostream>
#include <map>
#include <nlohmann/json.hpp>

#include <nlohmann/json.hpp>

namespace tol {

using json = nlohmann::json;

template <typename T> class StatsProps {
  virtual void increase(T value) = 0;
  virtual T get() const = 0;
  virtual void set(T stat) = 0;
  virtual std::ostream& print(std::ostream& print) const = 0;

  friend std::ostream& operator<<(std::ostream& stream, const StatsProps& stats) {
    return stats.print(stream);
  }
};

class Health: public StatsProps<size_t> {
  private:
  size_t health = 100;

  virtual std::ostream& print(std::ostream& out) const override;

  public:
  explicit Health(size_t health_);

  [[nodiscard]] size_t get() const override;
  void set(size_t stat) override;
  void increase(size_t value) override;
  void decrease(size_t value);
};

class Strength: public StatsProps<size_t> {
  size_t strength = 10;

  virtual std::ostream& print(std::ostream& out) const override;

  public:
  explicit Strength(size_t strength_);

  [[nodiscard]] size_t get() const override;
  void set(size_t stat) override;
  void increase(size_t value) override;
};

class Speed: public StatsProps<size_t> {
  size_t speed = 10;

  virtual std::ostream& print(std::ostream& out) const override;

  public:
  explicit Speed(size_t speed_);

  [[nodiscard]] size_t get() const override;
  void set(size_t stat) override;
  void increase(size_t value) override;
};

class Experience: public StatsProps<size_t> {
  size_t xp = 0;

  std::map<size_t, size_t> xp_bracket{ { 0, 1 },    { 100, 2 },  { 280, 3 },  { 500, 4 },  { 870, 5 },
                                       { 1300, 6 }, { 2000, 7 }, { 3000, 8 }, { 4500, 9 }, { 6600, 10 } };

  virtual std::ostream& print(std::ostream& out) const override;

  public:
  explicit Experience(size_t xp_);

  [[nodiscard]] size_t get() const override;
  void set(size_t stat) override;
  void increase(size_t value) override;

  [[nodiscard]] virtual size_t level() const;
};

class Stats {
  private:
  Health _health;
  Strength _strength;
  Speed _speed;
  Experience _experience;

  public:
  explicit Stats(const json& stats);

  [[nodiscard]] inline const Health& health() const {
    return _health;
  }
  [[nodiscard]] inline Health& health() {
    return _health;
  }

  [[nodiscard]] inline const Strength& strength() const {
    return _strength;
  }
  [[nodiscard]] inline Strength& strength() {
    return _strength;
  }

  [[nodiscard]] inline const Speed& speed() const {
    return _speed;
  }
  [[nodiscard]] inline Speed& speed() {
    return _speed;
  }

  [[nodiscard]] inline const Experience& experience() const {
    return _experience;
  }
  [[nodiscard]] inline Experience& experience() {
    return _experience;
  }

  [[nodiscard]] std::string get() const;
};

} // namespace tol
