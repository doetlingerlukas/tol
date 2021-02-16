#pragma once

#include <string>
#include <vector>

#include <object.hpp>

class Inventory {
  int max_size;
  std::vector<std::pair<std::string, Object>> elements;

public:
  explicit Inventory(int max_size_) : max_size(max_size_) {}

  int size() const {
    return elements.size();
  }

  std::vector<std::pair<std::string, Object>> getElements() const {
    return elements;
  }

  void add(std::pair<std::string, Object> new_element) {
    elements.push_back(new_element);
  }
};
