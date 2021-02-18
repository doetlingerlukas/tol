#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <overlay/info.hpp>
#include <protagonist.hpp>

class Quest {
  bool completed = false;

  protected:
  std::string name;
  std::string task;

  public:
  Quest(std::string name_, std::string task_) : name(std::move(name_)), task(std::move(task_)) {}

  virtual bool condition(Protagonist& player, Info& info) = 0;

  [[nodiscard]] std::string getName() const {
    return name;
  };

  [[nodiscard]] std::string getTask() const {
    return task;
  };

  void setCompleted() {
    completed = true;
  };

  // virtual void display_current(Info& info) const = 0;
};

class InitialQuest: public Quest {
  public:
  InitialQuest() : Quest("Gather resources!", "You are hungly. Find something to eat.") { }

  bool condition(Protagonist& player, Info& info) override {
    if (!player.getInventoryElements().empty()) {
      setCompleted();
      info.display_info(fmt::format("Completed Quest: {}", name), std::chrono::seconds(5));
      return true;
    }
    return false;
  }
};

class SearchQuest: public Quest {
  public:
  SearchQuest() : Quest("Find the lost item.", "<NPC> has lost something in the woods. Find it for him") { }

  bool condition(Protagonist& player, Info& info) override {
    if (false) {
      setCompleted();
      info.display_info(fmt::format("Completed Quest: {}", name), std::chrono::seconds(5));
      return true;
    }
    return false;
  }
};

class QuestStack {
  std::optional<int> selected;

  public:
  std::vector<std::unique_ptr<Quest>> quests;

  QuestStack() : selected(std::make_optional(0)) {
    quests.push_back(std::unique_ptr<Quest>(new InitialQuest()));
    quests.push_back(std::unique_ptr<Quest>(new SearchQuest()));
  }

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
