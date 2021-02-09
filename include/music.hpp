#pragma once

#include <SFML/Audio.hpp>

#include <filesystem>
#include <algorithm>

namespace tol {

class Music {
  sf::Music background;

  std::filesystem::path dir;

public:
  Music(std::filesystem::path dir_) : dir(dir_) {
    background.setLoop(true);
    background.setVolume(10.f);
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

  // Expects values between 0 and 1
  void set_volume(float volume) {
    background.setVolume(std::min(volume * 100.f, 100.f));
  }
};

}
