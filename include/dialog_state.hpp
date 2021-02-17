#pragma once

#include <string>

enum class DialogState { QUESTION, RESPONSE };

std::string stateAsString(DialogState state);

DialogState operator!(DialogState state);
