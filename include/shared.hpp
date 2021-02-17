#pragma once

constexpr float scale_ultra(const float x, const float y) {
  return (x / (x / y)) * 1.77777777778;
}

constexpr float
scale_pos(const float scalar, const float actual_res, const int resize_res, const float adjusted_offset) {
  return ((actual_res / resize_res) - 1.0f) < std::numeric_limits<float>::epsilon() ? scalar : adjusted_offset;
}
