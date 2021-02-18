#pragma once

#include <algorithm>
#include <filesystem>

#include <SFML/Audio.hpp>

namespace tol {

class Music {
  sf::Music background;

  std::filesystem::path dir;

  public:
  explicit Music(const std::filesystem::path& dir_, float volume): dir(dir_) {
    background.setLoop(true);
    background.setVolume(volume);
  }

  void play_background() {
    const auto music_path = (dir / "forgottenland.ogg").string();

    if (!background.openFromFile(music_path)) {
      throw std::runtime_error("Failed to load music '" + music_path + "'.");
    }

    background.play();
  }

  void stop_background() {
    background.stop();
  }

  void set_volume(float volume) {
    background.setVolume(std::clamp(volume, 0.f, 1.f) * 100.f);
  }
};

} // namespace tol
