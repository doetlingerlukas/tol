#pragma once

#include <memory>

#include <nuklear.hpp>
#include <dialog_state.hpp>
#include <game_state.hpp>

using json = nlohmann::json;

class Dialog {
private:
  std::shared_ptr<Nuklear> ui;
  json dialog;
  json init_npc_dialog;
  json dialog_progress;
  DialogState dialog_state = DialogState::QUESTION;

  json load() {
    std::ifstream ifs("assets/dialog.json");
    const auto& dialog_structure = json::parse(ifs);
    return dialog_structure["dialog"];
  }

public:
  GameState show(const std::string& character) {
    if(dialog_progress.is_null()) {
      init_npc_dialog = dialog_progress = dialog[character];

      if(dialog_progress.is_object()) {
        dialog_state = DialogState::RESPONSE;
        init_npc_dialog = dialog_progress[stateAsString(dialog_state)];
      }
    }

    if(dialog_progress.is_array()) {
      auto [progress, state] = ui->renderDialog(dialog_progress, dialog_state);
      dialog_progress = progress;
      dialog_state = state;
      return GameState::DIALOG;
    }

    if(dialog_progress.is_object()) {
      std::pair<json, DialogState> pair;

      if(dialog_progress[stateAsString(!dialog_state)].is_string()) {
        pair = ui->renderResponseDialog(dialog_progress, !dialog_state, init_npc_dialog);
        pair = make_pair(dialog_progress, !pair.second);
      } else {
        pair = ui->renderDialog(dialog_progress[stateAsString(!dialog_state)], dialog_state);
      }

      auto [progress, state] = pair;
      dialog_progress = progress;
      dialog_state = state;
      return GameState::DIALOG;
    }

    if(dialog_progress.is_string()) {
      auto [progress, state] = ui->renderResponseDialog(dialog_progress, dialog_state, init_npc_dialog);
      dialog_progress = progress;
      dialog_state = state;
      return GameState::DIALOG;
    }

    if(dialog_progress.is_number()) {
      int state = dialog_progress.get<int>();
      dialog_progress = nullptr;
      dialog_state = DialogState::QUESTION;
      return static_cast<GameState>(state);
    }

    return GameState::DIALOG;
  }

  Dialog(const std::shared_ptr<Nuklear> _ui) : ui(_ui), dialog(load()) { }
};
