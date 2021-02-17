#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <optional>

#include <overlay/info.hpp>
#include <protagonist.hpp>

class Quest {
  const std::string name;
  const std::string task;
  bool completed;

  public:
  Quest() {}
  Quest(const std::string& name_, const std::string& task_):
    name(name_), task(task_), completed(false) {}

  virtual bool condition(Protagonist& player, Info& info) = 0;

  const std::string& getTask() const {
    return task;
  }

  void setCompleted() {
    completed = true;
  }

  void display_current(Info& info) const {
    info.display_info("Quest: " + getTask(), std::chrono::seconds(10));
  }
};

class InitialQuest : public Quest {
  const std::string name = "Your first quest!";
  const std::string task = "You are hungly. Find something to eat.";
  bool completed = false;

  public:
  bool condition(Protagonist& player, Info& info) override {
    if (player.getInventoryElements().size() > 0) {
      setCompleted();
      info.display_info("Completed Quest: " + name, std::chrono::seconds(5));
      return true;
    }
    return false;
  }
};

class QuestStack {
  std::optional<int> selected;

  public:
  std::vector<std::unique_ptr<Quest>> quests;

  QuestStack() {}

  void select(int index) {
    if (quests.size() > index) {
      selected = index;
    }
  }

  void check(Protagonist& player, Info& info) {
    if (selected) {
      auto done = quests.at(static_cast<size_t>(*selected))->condition(player, info);
      if (done) {
        selected = std::nullopt;
      }
    }
  }
};