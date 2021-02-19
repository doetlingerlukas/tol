#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "overlay/info.hpp"

namespace tol {

class PlayState;

class Quest {
  std::string title_;
  std::string description_;
  std::function<bool(PlayState& player)> condition;
  bool completed_ = false;

  public:
  Quest(std::string title, std::string description, std::function<bool(PlayState& play_state)> condition);

  [[nodiscard]] inline const std::string& title() const {
    return title_;
  };

  [[nodiscard]] inline const std::string& description() const {
    return description_;
  };

  [[nodiscard]] inline bool completed() const {
    return completed_;
  }

  void check_condition(PlayState& play_state, Info& info);
};

class QuestStack {
  std::optional<int> selected;
  Info& info;

  public:
  std::vector<Quest> quests;

  explicit QuestStack(Info& info_);

  void select(size_t index);

  [[nodiscard]] int getSelected() const;

  void check(PlayState& play_state);

  bool completed(size_t index);
};

} // namespace tol
#ifndef TOL_PLAY_STATE_HPP
#include <play_state.hpp>
#endif
