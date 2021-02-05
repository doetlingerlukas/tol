#pragma once

class KeyInput {
public:
  bool up, down, left, right;
  bool w, a, s, d;

  KeyInput() {
    up = down = left = right = false;
    w = a = s = d = false;
  };
};