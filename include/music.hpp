#pragma once

#include <algorithm>
#include <filesystem>

#include <SFML/Audio.hpp>

namespace tol {

constexpr std::string_view default_music = "forgottenland.ogg";
constexpr std::string_view city_music = "adrift.ogg";

class Music {
  sf::Music background;

  std::filesystem::path dir;

  public:
  explicit Music(const std::filesystem::path& dir_, float volume): dir(dir_) {
    background.setLoop(true);
    background.setVolume(volume);
  }

  void play_background(const std::string_view music_) {
    background.stop();
    const auto music_path = (dir / music_).string();

    if (!background.openFromFile(music_path)) {
      throw std::runtime_error("Failed to load music '" + music_path + "'.");
    }

    background.play();
    background.setLoop(true);
  }

  void stop_background() {
    background.stop();
  }

  void set_volume(float volume) {
    background.setVolume(std::clamp(volume, 0.f, 1.f) * 100.f);
  }
};

} // namespace tol
