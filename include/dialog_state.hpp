#pragma once

#include <string>

namespace tol {

enum class DialogState { QUESTION, RESPONSE };

std::string stateAsString(DialogState state);

DialogState operator!(DialogState state);

} // namespace tol
