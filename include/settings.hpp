#pragma once

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

class Settings {
private:
  int resolution_height;
  int resolution_width;
  bool is_fullscreen;
  fs::path settings_path;

  void initSettings() const {
    using json = nlohmann::json;

    if (fs::exists(settings_path)) {
      return;
    }

    json settings;
    auto& settings_field = settings["settings"];
    auto& resolution = settings_field["resolution"];
    resolution["width"] = 1200;
    resolution["height"] = 800;
    settings_field["fullscreen"] = false;
    std::cout << std::setw(2) << settings << std::endl;

    std::ofstream out(settings_path);
    out << std::setw(2) << settings << std::endl;
  }

  void loadSettings() {
    using json = nlohmann::json;

    initSettings();
    std::ifstream ifs(settings_path);
    json settings = json::parse(ifs);

    const auto& settings_field = settings["settings"];
    const auto& resolution = settings_field["resolution"];
    resolution_height = resolution["height"].get<int>();
    resolution_width = resolution["width"].get<int>();
    is_fullscreen = settings_field["fullscreen"].get<bool>();
    std::cout << std::setw(2) << settings << std::endl;
  }

public:
  Settings(const fs::path exec_path) : settings_path(fs::canonical(exec_path).parent_path() / "settings.json") {
    loadSettings();
  }

  std::tuple<int, int> resolution() const {
    return std::make_tuple(resolution_width, resolution_height);
  }

  bool fullscreen() const {
    return is_fullscreen;
  }
};
