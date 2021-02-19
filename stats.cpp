#include <sstream>

#include "stats.hpp"

namespace tol {

size_t Health::get() const {
  return health;
}

void Health::set(size_t stat) {
  health = stat;
}

void Health::increase(size_t value) {
  health = std::clamp<size_t>(health + value, 0, 100);
}

void Health::decrease(size_t value) {
  health = std::clamp<int>(health - value, 0, 100);
}

Health::Health(size_t health_): health(health_) {}

std::ostream& Health::print(std::ostream& out) const {
  out << "Health: " << health << "\n";
  return out;
}

Strength::Strength(size_t strength_): strength(strength_) {}

size_t Strength::get() const {
  return strength;
}

void Strength::set(size_t stat) {
  strength = stat;
}

void Strength::increase(size_t value) {
  strength += value;
}

std::ostream& Strength::print(std::ostream& out) const {
  out << "Strength: " << strength << "\n";
  return out;
}

Speed::Speed(size_t speed_): speed(speed_) {}

size_t Speed::get() const {
  return speed;
}

void Speed::set(size_t stat) {
  speed = stat;
}

void Speed::increase(size_t value) {
  speed += value;
}

std::ostream& Speed::print(std::ostream& out) const {
  out << "Speed: " << speed << "\n";
  return out;
}

Experience::Experience(size_t xp_): xp(xp_) {}

size_t Experience::get() const {
  return xp;
}

void Experience::set(size_t stat) {
  xp = stat;
}

void Experience::increase(size_t value) {
  xp += value;
}

size_t Experience::level() const {
  auto level = 0;

  for (const auto& [xp_, level_]: xp_bracket) {
    if (xp >= xp_) {
      level = level_;
      continue;
    }

    break;
  }

  return level;
}

std::ostream& Experience::print(std::ostream& out) const {
  out << "Experience: " << xp << ", Level: " << level() << "\n";
  return out;
}

Stats::Stats(const json& stats):
  _health(Health(stats["health"].get<size_t>())), _strength(Strength(stats["strength"].get<size_t>())),
  _speed(Speed(stats["speed"].get<size_t>())), _experience(Experience(stats["experience"].get<size_t>())) {}

std::string Stats::get() const {
  std::stringstream ss;
  ss << _health;
  ss << _strength;
  ss << _speed;
  ss << _experience;
  return ss.str();
}

} // namespace tol
