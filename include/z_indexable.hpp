#pragma once

#include <optional>

namespace tol {

class ZIndexable {
  public:
  [[nodiscard]] virtual std::optional<float> zIndex() const = 0;
};

} // namespace tol
