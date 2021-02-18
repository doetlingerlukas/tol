#pragma once

namespace tol {

class KeyInput {
  public:
  bool up, down, left, right;
  bool w, a, s, d;
  bool e;
  bool enter;

  KeyInput() {
    up = down = left = right = false;
    w = a = s = d = false;
    e = false;
    enter = false;
  };
};

} // namespace tol
