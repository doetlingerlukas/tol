#pragma once

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Settings {
  int resolution_height;
  int resolution_width;
  bool is_fullscreen;
  bool vsync_enabled;
  fs::path settings_path;

  template <typename T>
  T get_or_else(json structure, std::string field, T default_value) {
    if(structure.contains(field)) {
      return structure[field].get<T>();
    } else {
      return default_value;
    }
  }

  void loadSettings() {
    json settings;

    if (fs::exists(settings_path)) {
      std::ifstream ifs(settings_path);
      settings = json::parse(ifs);
    }

    auto& settings_field = settings["settings"];
    auto& resolution = settings_field["resolution"];

    const int width = get_or_else<int>(resolution, "width", 1200);
    resolution["width"] = width;
    resolution_width = width;

    const int height = get_or_else<int>(resolution, "height", 800);
    resolution["height"] = height;
    resolution_height = height;

    const bool fullscreen = get_or_else<bool>(settings_field, "fullscreen", false);
    settings_field["fullscreen"] = fullscreen;
    is_fullscreen = fullscreen;

    const bool vsync = get_or_else<bool>(settings_field, "vsync", false);
    settings_field["vsync"] = vsync;
    vsync_enabled = vsync;

    const float volume = get_or_else<float>(settings_field, "volume", 0.1f);
    settings_field["volume"] = volume;
    volume_level = volume;

    std::cout << std::setw(2) << settings << std::endl;

    std::ofstream out(settings_path);
    out << std::setw(2) << settings << std::endl;
  }

public:
  float volume_level;

  Settings(const fs::path exec_path) : settings_path(fs::canonical(exec_path).parent_path() / "settings.json") {
    loadSettings();
  }

  std::tuple<int, int> resolution() const {
    return std::make_tuple(resolution_width, resolution_height);
  }

  bool fullscreen() const {
    return is_fullscreen;
  }

  bool vsync() const {
    return vsync_enabled;
  }

  void set_fullscreen(bool value) {
    is_fullscreen = value;
  }

  void set_vsync(bool value) {
    vsync_enabled = value;
  }
};
