#include <stats.hpp>

void Health::subscribe(std::function<void()> func) {
  callback = func;
}

void Health::increase(size_t value) {
  health = std::clamp<size_t>(0, 100, health + value);
}

size_t Health::get() const {
  return health;
}

void Health::decrease(size_t value) {
  if (health == 0 || health == value) {
    health = 0;
    callback();
    return;
  }

  health = health - value;
}

Health::Health(size_t health_) : health (health_), future_obj(exit_signal.get_future()), regen_thread([this] (std::future<void> future) {
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

Health::~Health() {
  exit_signal.set_value();
  regen_thread.join();
}

std::ostream& Health::print(std::ostream& out) const {
  out << "health: " << health << "\n";
  return out;
}


Strength::Strength(size_t strength_) : strength(static_cast<int>(strength_)) { }

void Strength::increase(size_t value) {
  strength += static_cast<int>(value);
}

size_t Strength::get() const {
  return strength;
}

std::ostream& Strength::print(std::ostream& out) const {
  out << "strength: " << strength << "\n";
  return out;
}

Experience::Experience(size_t lvl) : level(lvl) { }

void Experience::increase(size_t value) {
  experience += value;

  for(auto& [xp, lvl]: xp_bracket) {
    if(xp > experience) {
      break;
    }

    level = lvl;
  }
}

Speed::Speed(size_t speed_) : speed(static_cast<int>(speed_)) { }

void Speed::increase(size_t value) {
  speed += static_cast<int>(value);
}

size_t Speed::get() const {
  return speed;
}

std::ostream& Speed::print(std::ostream& out) const {
  out << "speed: " << speed << "\n";
  return out;
}

size_t Experience::get() const {
  return experience;
}

size_t Experience::getLevel() const {
  return level;
}

std::ostream& Experience::print(std::ostream& out) const {
  out << "experience: " << experience << ", level: " << level << "\n";
  return out;
}


Stats::Stats(const json& stats):
  _health(Health(stats["health"].get<size_t>())),
  _strength(Strength(stats["strength"].get<size_t>())),
  _speed(Speed(stats["speed"].get<size_t>())),
  _experience(Experience(stats["level"].get<size_t>())) { }

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

void Stats::get() {
  std::cout << _health;
  std::cout << _strength;
  std::cout << _speed;
  std::cout << _experience;
}
