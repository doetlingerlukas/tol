#pragma once

#include <optional>

class ZIndexable {
public:
  virtual std::optional<float> zIndex() const = 0;
};
