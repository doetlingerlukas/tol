#pragma once

#include <chrono>

class Animation {
  std::chrono::milliseconds start_time = std::chrono::milliseconds(0);
  size_t current = 0;
  std::vector<std::tuple<std::chrono::milliseconds, tson::Rect>> frames;

public:
  Animation(const std::vector<tson::Frame>& frames, tson::Tileset* tileset) {
    for (const auto& frame: frames) {
      const auto tile_id = frame.getTileId();
      const auto rect = tileset->getTile(tile_id)->getDrawingRect();
      this->frames.push_back(std::make_tuple(std::chrono::milliseconds(frame.getDuration()), rect));
    }
  }

  tson::Rect getDrawingRect(std::chrono::milliseconds ms) {
    const auto& [duration, rect] = frames.at(current);

    if (start_time == std::chrono::milliseconds(0)) {
      start_time = ms;
    } else if (ms > start_time + duration) {
      current = (current + 1) % frames.size();
      start_time = ms;

      return std::get<1>(frames.at(current));
    }

    return rect;
  }
};
