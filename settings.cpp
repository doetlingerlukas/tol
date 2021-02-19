#include "settings.hpp"

namespace tol {

void Settings::load_settings() {
  json settings;

  if (fs::exists(settings_path)) {
    std::ifstream ifs(settings_path);
    settings = json::parse(ifs);
  }

  auto& settings_field = settings["settings"];
  auto& resolution = settings_field["resolution"];

  const int width = get_or_else(resolution, "width", 1200);
  resolution["width"] = width;
  resolution_width = width;

  const int height = get_or_else(resolution, "height", 800);
  resolution["height"] = height;
  resolution_height = height;

  const bool fullscreen = get_or_else(settings_field, "fullscreen", false);
  settings_field["fullscreen"] = fullscreen;
  is_fullscreen = fullscreen;

  const bool vsync = get_or_else(settings_field, "vsync", false);
  settings_field["vsync"] = vsync;
  vsync_enabled = vsync;

  const float volume = get_or_else(settings_field, "volume", 0.1f);
  settings_field["volume"] = volume;
  volume_level = volume;

  std::cout << std::setw(2) << settings << std::endl;

  std::ofstream out(settings_path);
  out << std::setw(2) << settings << std::endl;
}

Settings::Settings(const fs::path exec_path): settings_path(fs::canonical(exec_path).parent_path() / "settings.json") {
  load_settings();
}

std::pair<int, int> Settings::resolution() const {
  return std::make_pair(resolution_width, resolution_height);
}

void Settings::set_resolution(std::tuple<int, int> res) {
  const auto [width, height] = res;
  resolution_width = width;
  resolution_height = height;
}

void Settings::set_resolution(sf::VideoMode video_mode) {
  resolution_width = video_mode.width;
  resolution_height = video_mode.height;
}

bool Settings::fullscreen() const {
  return is_fullscreen;
}

bool Settings::vsync() const {
  return vsync_enabled;
}

void Settings::set_fullscreen(bool value) {
  is_fullscreen = value;
}

void Settings::set_vsync(bool value) {
  vsync_enabled = value;
}

void Settings::serialize() const {
  json settings;

  settings["settings"]["resolution"]["width"] = resolution_width;
  settings["settings"]["resolution"]["height"] = resolution_height;
  settings["settings"]["fullscreen"] = is_fullscreen;
  settings["settings"]["vsync"] = vsync_enabled;
  settings["settings"]["volume"] = volume_level;

  std::ofstream ofs(settings_path, std::ofstream::out | std::ofstream::trunc);
  ofs << settings.dump(2);
  ofs.close();
}

} // namespace tol
