#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

namespace tol {

namespace fs = std::filesystem;

using json = nlohmann::json;

constexpr std::array<std::pair<int, int>, 4> supported_resolutions = {
  std::make_pair(3840, 1600), std::make_pair(3440, 1440), std::make_pair(2560, 1440), std::make_pair(1920, 1080)
};

template <typename T> T get_or_else(json structure, std::string field, T default_value) {
  if (structure.contains(field)) {
    return structure[field].get<T>();
  } else {
    return default_value;
  }
}

class Settings {
  int resolution_height;
  int resolution_width;
  bool is_fullscreen;
  bool vsync_enabled;
  fs::path settings_path;

  void load_settings();

  public:
  float volume_level;

  explicit Settings(fs::path exec_path);

  [[nodiscard]] std::pair<int, int> resolution() const;

  void set_resolution(std::tuple<int, int> res);

  void set_resolution(sf::VideoMode video_mode);

  [[nodiscard]] bool fullscreen() const;

  [[nodiscard]] bool vsync() const;

  void set_fullscreen(bool value);

  void set_vsync(bool value);

  void serialize() const;
};

} // namespace tol
