#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <overlay/info.hpp>
#include <protagonist.hpp>

class Quest {
  public:
  Quest() {}

  virtual bool condition(Protagonist& player, Info& info) = 0;

  virtual const std::string& getName() const = 0;

  virtual const std::string& getTask() const = 0;

  virtual void setCompleted() = 0;

  // virtual void display_current(Info& info) const = 0;
};

class InitialQuest: public Quest {
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

  const std::string& getName() const override {
    return name;
  }

  const std::string& getTask() const override {
    return task;
  }

  void setCompleted() override {
    completed = true;
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

  int getSelected() const {
    return selected.value_or(-1);
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