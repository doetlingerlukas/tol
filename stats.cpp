#include <sstream>

#include "stats.hpp"

namespace tol {

void Health::subscribe(std::function<void()> func) {
  callback = func;
}

void Health::increase(size_t value) {
  health = std::clamp<size_t>(health + value, 0, 100);
}

size_t Health::get() const {
  return health;
}

void Health::set(size_t stat) {
  health = stat;
}

void Health::decrease(size_t value) {
  health = std::clamp<int>(health - value, 0, 100);

  if (health == 0 && callback != nullptr)
    callback();
}

Health::Health(size_t health_): health(health_) {}

std::ostream& Health::print(std::ostream& out) const {
  out << "Health: " << health << "\n";
  return out;
}

Strength::Strength(size_t strength_): strength(strength_) {}

void Strength::increase(size_t value) {
  strength += value;
}

size_t Strength::get() const {
  return strength;
}

void Strength::set(size_t stat) {
  strength = stat;
}

std::ostream& Strength::print(std::ostream& out) const {
  out << "Strength: " << strength << "\n";
  return out;
}

Experience::Experience(size_t lvl): level(lvl) {}

void Experience::increase(size_t value) {
  experience += value;

  for (auto& [xp, lvl]: xp_bracket) {
    if (xp > experience) {
      break;
    }

    level = lvl;
  }
}

Speed::Speed(size_t speed_): speed(speed_) {}

void Speed::increase(size_t value) {
  speed += value;
}

size_t Speed::get() const {
  return speed;
}

void Speed::set(size_t stat) {
  speed = stat;
}

std::ostream& Speed::print(std::ostream& out) const {
  out << "Speed: " << speed << "\n";
  return out;
}

size_t Experience::get() const {
  return experience;
}

size_t Experience::getLevel() const {
  return level;
}

void Experience::set(size_t stat) {
  experience = stat;
}

void Experience::setAll(size_t xp, size_t lvl) {
  experience = xp;
  level = lvl;
}

std::ostream& Experience::print(std::ostream& out) const {
  out << "Experience: " << experience << ", Level: " << level << "\n";
  return out;
}

Stats::Stats(const json& stats):
  _health(Health(stats["health"].get<size_t>())), _strength(Strength(stats["strength"].get<size_t>())),
  _speed(Speed(stats["speed"].get<size_t>())), _experience(Experience(stats["level"].get<size_t>())) {}

Health& Stats::health() {
  return _health;
}

Strength& Stats::strength() {
  return _strength;
}

Speed& Stats::speed() {
  return _speed;
}

Experience& Stats::experience() {
  return _experience;
}

std::string Stats::get() {
  std::stringstream ss;
  ss << _health;
  ss << _strength;
  ss << _speed;
  ss << _experience;
  return ss.str();
}

} // namespace tol
