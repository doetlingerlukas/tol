#pragma once

#include <string>
#include <vector>

#include <object.hpp>

class Inventory {
  int max_size;
  std::vector<std::pair<std::string, Object>> elements;

  public:
  explicit Inventory(int max_size_): max_size(max_size_) {}

  int size() const {
    return elements.size();
  }

  std::vector<std::pair<std::string, Object>> getElements() const {
    return elements;
  }

  bool add(std::pair<std::string, Object> new_element) {
    if (size() < max_size) {
      elements.push_back(new_element);
      return true;
    }

    return false;
  }

  std::pair<std::string, Object> remove(size_t index) {
    return *elements.erase(elements.cbegin() + index);
  }
};
