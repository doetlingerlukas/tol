#pragma once

#include <string>
#include <vector>
#include <chrono>

#include <overlay/info.hpp>

class Quest {
  const std::string name;
  bool completed;

  std::vector<std::string> tasks;

  public:
  Quest(const std::string& name_, std::vector<std::string> tasks_):
    name(name_), tasks(tasks_), completed(false) {}

  void add_task(const std::string& desc) {
    tasks.push_back(desc);
  }

  const std::string& currentTask() const {
    return tasks.front();
  }

  std::string advance() {
    return *tasks.erase(tasks.cbegin());
  }

  void setCompleted() {
    completed = true;
  }

  void display_current(Info& info) const {
    info.display_info("Quest: " + currentTask(), std::chrono::seconds(10));
  }
};