#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <overlay/info.hpp>

class Protagonist;

class Quest {
  std::string title_;
  std::string description_;
  std::function<bool(Protagonist& player)> condition;
  bool completed_ = false;

  public:
  Quest(std::string title, std::string description, std::function<bool(Protagonist& player)> condition);

  [[nodiscard]] inline const std::string& title() const {
    return title_;
  };

  [[nodiscard]] inline const std::string& description() const {
    return description_;
  };

  [[nodiscard]] inline bool completed() {
    return completed_;
  }

  void check_condition(Protagonist& player, Info& info);
};

class QuestStack {
  std::optional<int> selected;
  Info& info;

  public:
  std::vector<Quest> quests;

  QuestStack(Info& info_);

  void select(int index);

  [[nodiscard]] int getSelected() const;

  void check(Protagonist& player);

  bool completed(size_t index);
};

#ifndef TOL_PROTAGONIST_HPP
#include <protagonist.hpp>
#endif
