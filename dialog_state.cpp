#include "dialog_state.hpp"

namespace tol {

std::string stateAsString(DialogState state) {
  switch (state) {
    case DialogState::QUESTION:
      return "question";
    case DialogState::RESPONSE:
      return "response";
  }

  return "question";
}

DialogState operator!(DialogState state) {
  return state == DialogState::QUESTION ? DialogState::RESPONSE : DialogState::QUESTION;
}

} // namespace tol
