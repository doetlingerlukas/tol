#pragma once

enum class DialogState {
  QUESTION,
  RESPONSE
};

extern std::string stateAsString(DialogState state) {
  switch(state) {
    case DialogState::QUESTION:
      return "question";
    case DialogState::RESPONSE:
      return "response";
  }

  return "question";
}

extern DialogState operator!(DialogState state) {
  return state == DialogState::QUESTION ? DialogState::RESPONSE : DialogState::QUESTION;
}

