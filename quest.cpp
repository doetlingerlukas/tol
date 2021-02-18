#include "quest.hpp"

namespace tol {

Quest::Quest(std::string title, std::string description, std::function<bool(Protagonist& player)> condition_):
  title_(std::move(title)), description_(std::move(description)), condition(std::move(condition_)) {}

void Quest::check_condition(Protagonist& player, Info& info) {
  if (completed()) {
    return;
  }

  if (condition(player)) {
    completed_ = true;
    info.display_info(fmt::format("Completed Quest: {}", title()), std::chrono::seconds(5));
  }
}

QuestStack::QuestStack(Info& info_): selected(std::make_optional(0)), info(info_) {
  quests.push_back(Quest("Gather resources!", "You are hungly. Find something to eat.", [](auto player) {
    return !player.getInventoryElements().empty();
  }));
  quests.push_back(
    Quest("Find the lost item.", "<NPC> has lost something in the woods. Find it for him", [](auto player) {
      return false;
    }));
}

void QuestStack::select(int index) {
  if (quests.size() > index) {
    selected = index;
    const auto& quest = quests[index];
    info.display_info(fmt::format("Active Quest: {}", quest.title()), std::chrono::seconds(5));
  }
}

[[nodiscard]] int QuestStack::getSelected() const {
  return selected.value_or(-1);
}

void QuestStack::check(Protagonist& player) {
  if (selected) {
    quests.at(static_cast<size_t>(*selected)).check_condition(player, info);
  }
}

bool QuestStack::completed(size_t index) {
  return quests[index].completed();
}

} // namespace tol
