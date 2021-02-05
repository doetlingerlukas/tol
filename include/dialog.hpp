#pragma once

#include <memory>

#include <nuklear.hpp>

using json = nlohmann::json;

class Dialog {
private:
  std::shared_ptr<Nuklear> ui;
  json dialog;

  static json load() {
    std::ifstream ifs("assets/dialog.json");
    const auto& dialog_structure = json::parse(ifs);
    return dialog_structure["dialog"];
  }

public:
  void show(std::string character) const {
    ui->renderDialog(dialog[character]);
  }

  Dialog(const std::shared_ptr<Nuklear> _ui) : ui(_ui), dialog(load()) { }
};
