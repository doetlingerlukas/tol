#pragma once

#include <algorithm>
#include <filesystem>

#include <SFML/Audio.hpp>

namespace tol {

constexpr std::string_view default_music = "forgottenland.ogg";
constexpr std::string_view city_music = "adrift.ogg";

class Music {
  sf::Music default_background;
  sf::Music city_background;

  std::filesystem::path dir;
  float volume;
  bool entered_city = false;
  bool is_default = false;

  static void load_audio_file(sf::Music& music, const std::string& path) {
    if (!music.openFromFile(path)) {
      throw std::runtime_error("Failed to load music '" + path + "'.");
    }
  }

  public:
  explicit Music(const std::filesystem::path& dir_, float volume_): dir(dir_) {
    load_audio_file(default_background, (dir / default_music).string());
    load_audio_file(city_background, (dir / city_music).string());
    set_volume(volume_);
  }

  void play_default() {
    if (!is_default) {
      city_background.stop();

      default_background.play();
      default_background.setLoop(true);
    }
    entered_city = false;
    is_default = true;
  }

  void play_city() {
    if (!entered_city) {
      default_background.stop();

      city_background.play();
      city_background.setLoop(true);
    }
    entered_city = true;
    is_default = false;
  }

  void set_volume(float volume_) {
    volume = std::clamp(volume_, 0.f, 1.f) * 100.f;
    default_background.setVolume(volume);
    city_background.setVolume(volume);
  }
};

} // namespace tol
