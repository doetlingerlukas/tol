#pragma once

#include <optional>

namespace tol {

class ZIndexable {
  public:
  [[nodiscard]] virtual std::optional<float> z_index() const = 0;
};

} // namespace tol
