#pragma once

#include <memory>

#include <nuklear.hpp>

using json = nlohmann::json;

class Dialog {
private:
  std::shared_ptr<Nuklear> ui;
  json dialog;

  static json load() {
    std::ifstream ifs("dialog.json");
    const auto& dialog_structure = json::parse(ifs);
    return dialog_structure["dialog"];
  }

public:
  void show(std::string character) const {
    std::vector<std::string> dialog_lines;

    for(const auto& line : dialog[character]) {
      dialog_lines.push_back(line["question"].get<std::string>());
    }

    ui->renderDialog(dialog_lines);
  }

  Dialog(const std::shared_ptr<Nuklear> _ui) : ui(_ui), dialog(load()) { }
};
