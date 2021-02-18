#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <overlay/info.hpp>
#include <protagonist.hpp>

class Quest {
  public:
  Quest() = default;

  virtual bool condition(Protagonist& player, Info& info) = 0;

  [[nodiscard]] virtual const std::string& getName() const = 0;

  [[nodiscard]] virtual const std::string& getTask() const = 0;

  virtual void setCompleted() = 0;

  // virtual void display_current(Info& info) const = 0;
};

class InitialQuest: public Quest {
  const std::string name = "Gather resources!";
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

  [[nodiscard]] const std::string& getName() const override {
    return name;
  }

  [[nodiscard]] const std::string& getTask() const override {
    return task;
  }

  void setCompleted() override {
    completed = true;
  }
};

class SearchQuest: public Quest {
  const std::string name = "Find the lost item.";
  const std::string task = "<NPC> has lost something in the woods. Find it for him to receive a reward.";
  bool completed = false;

  public:
  bool condition(Protagonist& player, Info& info) override {
    return false;
  }

  [[nodiscard]] const std::string& getName() const override {
    return name;
  }

  [[nodiscard]] const std::string& getTask() const override {
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

  QuestStack() = default;

  void select(int index) {
    if (quests.size() > index) {
      selected = index;
    }
  }

  [[nodiscard]] int getSelected() const {
    return selected.value_or(-1);
  }

  void check(Protagonist& player, Info& info) {
    if (selected) {
      auto done = quests.at(static_cast<size_t>(*selected))->condition(player, info);
      if (done) {
        quests.erase(quests.cbegin() + *selected);
        selected = std::nullopt;
      }
    }
  }
};