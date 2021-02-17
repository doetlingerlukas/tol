#pragma once

#include <optional>

class ZIndexable {
  public:
  [[nodiscard]] virtual std::optional<float> zIndex() const = 0;
};
