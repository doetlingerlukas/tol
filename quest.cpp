#include "quest.hpp"

namespace tol {

Quest::Quest(std::string title, std::string description, std::function<bool(PlayState& play_state)> condition_):
  title_(std::move(title)), description_(std::move(description)), condition(std::move(condition_)) {}

void Quest::check_condition(PlayState& play_state, Info& info) {
  if (completed()) {
    return;
  }

  if (condition(play_state)) {
    completed_ = true;
    info.display_info(fmt::format("Completed Quest: {}", title()), std::chrono::seconds(10));
  }
}

QuestStack::QuestStack(Info& info_): selected(std::make_optional(0)), info(info_) {
  quests.push_back(Quest("Gather resources!", "You are hungry. Find something to eat.", [](auto play_state) {
    return !play_state.player().inventory().empty();
  }));
  quests.push_back(
    Quest("Find the lost item.", "Detlef has lost something in the woods. Find it for him.", [](auto play_state) {
      auto& map = play_state.map();

      auto& npc = map.getNpc("Detlef de Loost");

      auto pair = map.getCollectible("tools");
      if (pair) {
        auto& [id, collectible] = *pair;
        return collectible.collides_with(npc.getBoundingRect());
      }

      return false;
    }));
  quests.push_back(Quest("Get a baguette.", "Find a baguette to stab your last opponent.", [](auto play_state) {
    const auto& items = play_state.player().inventory().items();
    return std::any_of(
      items.cbegin(), items.cend(), [](const auto& item) { return item.second.getName() == "baguette"; });
  }));
}

void QuestStack::select(size_t index) {
  if (quests.size() > index) {
    selected = index;
    const auto& quest = quests[index];
    info.display_info(fmt::format("Active Quest: {}", quest.title()), std::chrono::seconds(5));
  }
}

[[nodiscard]] int QuestStack::getSelected() const {
  return selected.value_or(-1);
}

void QuestStack::check(PlayState& play_state) {
  if (selected) {
    quests.at(static_cast<size_t>(*selected)).check_condition(play_state, info);
  }
}

bool QuestStack::completed(size_t index) const {
  return quests[index].completed();
}

void QuestStack::setCompleted(size_t index) const {
  quests[index].setCompleted();
}

[[nodiscard]] size_t QuestStack::count() const {
  return quests.size();
}

} // namespace tol
