#pragma once

#include <chrono>

#include <SFML/Graphics.hpp>
#include <tileson.hpp>

namespace tol {

class Animation {
  std::chrono::milliseconds total_time = std::chrono::milliseconds(0);
  std::vector<std::tuple<std::chrono::milliseconds, sf::IntRect>> frames;

  public:
  Animation(std::vector<std::tuple<std::chrono::milliseconds, sf::IntRect>>&& frames) {
    for (const auto& frame: frames) {
      total_time += std::get<0>(frame);
    }

    this->frames = frames;
  }

  Animation(const std::vector<tson::Frame>& frames, tson::Tileset& tileset) {
    for (const auto& frame: frames) {
      const auto tile_id = frame.getTileId();
      const auto rect = tileset.getTile(tile_id)->getDrawingRect();
      const auto duration = std::chrono::milliseconds(frame.getDuration());
      total_time += duration;
      this->frames.emplace_back(duration, sf::IntRect{ rect.x, rect.y, rect.width, rect.height });
    }
  }

  sf::IntRect drawing_rect(std::chrono::milliseconds ms) const {
    std::chrono::milliseconds current = ms % total_time;

    std::chrono::milliseconds now = std::chrono::milliseconds(0);
    for (const auto& frame: frames) {
      const auto duration = std::get<0>(frame);

      if (current >= now && current <= now + duration) {
        return std::get<1>(frame);
      }

      now += duration;
    }

    throw std::runtime_error("failed to select animation frame");
  }
};

}
