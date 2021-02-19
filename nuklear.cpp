#include "nuklear.hpp"

namespace tol {

struct nk_context* Nuklear::init(sf::RenderWindow* window) {
  struct nk_context* ctx;
  ctx = nk_sfml_init(window);
  return ctx;
}

void Nuklear::push_window_state() const {
  window->pushGLStates();
}

void Nuklear::pop_window_state() const {
  nk_sfml_render(NK_ANTI_ALIASING_ON);
  window->popGLStates();
}

void Nuklear::render_death(GameInstance& game, PlayState& play_state) const {
  auto ctx = this->ctx();

  push_window_state();
  const float button_height = 40;
  const int r = 0;
  const int g = 0;
  const int b = 0;
  const int a = 0;

  struct nk_color background = nk_rgba(r, g, b, a);

  const auto* font = asset_cache->load_nk_font("fonts/Gaegu-Regular.ttf", 24 * scale.y);
  nk_style_set_font(ctx, &font->handle);

  struct nk_style& s = ctx->style;

  const float spacing = 10;

  nk_style_push_color(ctx, &s.window.background, background);
  nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(r, g, b, a)));
  nk_style_push_style_item(ctx, &s.button.normal, nk_style_item_color(nk_rgba(40, 40, 40, 255)));
  nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(50, 50, 50, 255)));
  nk_style_push_style_item(ctx, &s.button.active, nk_style_item_color(nk_rgba(30, 30, 30, 255)));
  nk_style_push_color(ctx, &s.button.text_normal, nk_rgba(255, 255, 255, 255));
  nk_style_push_color(ctx, &s.button.text_hover, nk_rgba(255, 255, 255, 255));
  nk_style_push_color(ctx, &s.button.text_active, nk_rgba(255, 255, 255, 255));
  nk_style_push_vec2(ctx, &s.window.spacing, nk_vec2(0, spacing * scale.y));
  ctx->style.button.text_alignment = NK_TEXT_CENTERED;

  /* GUI */
  if (nk_begin(
        ctx, "death", nk_rect(0, 0, static_cast<float>(size.x), static_cast<float>(size.y)), NK_WINDOW_BACKGROUND)) {
    static const std::array<float, 3> ratio{ 0.3f, 0.4f, 0.3f };

    nk_layout_row_static(ctx, (size.y / 3.0f - 32 * scale.y), 0, 1);
    nk_layout_row(ctx, NK_DYNAMIC, 32, 2, ratio.data());

    nk_spacing(ctx, 1);

    nk_label(ctx, "YOU ARE DEAD", NK_TEXT_CENTERED);

    nk_layout_row_static(ctx, button_height, 0, 1);
    nk_layout_row(ctx, NK_DYNAMIC, button_height * scale.y, 2, ratio.data());

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "LOAD")) {
      game.load_position(play_state);
      game.setState(GameState::PLAY);
    }

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "EXIT")) {
      game.setState(GameState::QUIT);
    }
  }

  nk_end(ctx);
  nk_style_pop_vec2(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_color(ctx);
  pop_window_state();
}

void Nuklear::render_menu(
  GameInstance& game, PlayState& play_state, const Character& player, const Inventory& inventory,
  QuestStack& quests) const {
  auto ctx = this->ctx();

  push_window_state();
  const float button_height = 40;
  const int r = 0;
  const int g = 0;
  const int b = 0;
  const int a = 0;

  struct nk_color background = nk_rgba(r, g, b, a);

  const auto* font = asset_cache->load_nk_font("fonts/Gaegu-Regular.ttf", 32 * scale.y);
  nk_style_set_font(ctx, &font->handle);

  struct nk_style& s = ctx->style;

  const float spacing = 10;

  nk_style_push_color(ctx, &s.window.background, background);
  nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(r, g, b, a)));
  nk_style_push_style_item(ctx, &s.button.normal, nk_style_item_color(nk_rgba(40, 40, 40, 255)));
  nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(50, 50, 50, 255)));
  nk_style_push_style_item(ctx, &s.button.active, nk_style_item_color(nk_rgba(30, 30, 30, 255)));
  nk_style_push_color(ctx, &s.button.text_normal, nk_rgba(255, 255, 255, 255));
  nk_style_push_color(ctx, &s.button.text_hover, nk_rgba(255, 255, 255, 255));
  nk_style_push_color(ctx, &s.button.text_active, nk_rgba(255, 255, 255, 255));
  nk_style_push_vec2(ctx, &s.window.spacing, nk_vec2(0, spacing * scale.y));
  ctx->style.button.text_alignment = NK_TEXT_CENTERED;

  /* GUI */
  if (nk_begin(
        ctx, "menu", nk_rect(0, 0, static_cast<float>(size.x), static_cast<float>(size.y)), NK_WINDOW_BACKGROUND)) {
    static const std::array<float, 3> ratio{ 0.3f, 0.4f, 0.3f };

    nk_layout_row_static(ctx, (size.y - (button_height * 5.f + spacing * 6.f) * scale.y) / 2.f, 0, 1);
    nk_layout_row(ctx, NK_DYNAMIC, button_height * scale.y, 2, ratio.data());

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "CONTINUE")) {
      game.setState(GameState::PLAY);
    }

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "LOAD")) {
      game.load(quests, play_state);
      game.setState(GameState::PLAY);
    }

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "SAVE")) {
      game.save(play_state, player, inventory, quests);
      game.setState(GameState::PLAY);
    }

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "SETTINGS")) {
      game.setState(GameState::SETTINGS);
    }

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "EXIT")) {
      game.setState(GameState::QUIT);
    }
  }

  nk_end(ctx);
  nk_style_pop_vec2(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_color(ctx);
  pop_window_state();
}

void Nuklear::render_settings(GameInstance& game, Settings& settings) {
  auto ctx = this->ctx();

  push_window_state();
  auto video_mode = sf::VideoMode::getDesktopMode();
  const float setting_height = 40;
  const float space = 10;

  auto res_to_string = [](std::pair<int, int> res) {
    const auto [width, height] = res;
    return std::to_string(width) + " x " + std::to_string(height);
  };

  static int selected_res = 3;

  struct nk_style& s = ctx->style;

  const auto* font = asset_cache->load_nk_font("fonts/Gaegu-Regular.ttf", 32 * scale.y);
  nk_style_set_font(ctx, &font->handle);

  nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
  nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(50, 50, 50, 255)));
  nk_style_push_color(ctx, &s.button.text_hover, nk_rgba(255, 255, 255, 255));
  nk_style_push_vec2(ctx, &s.window.spacing, nk_vec2(0, space * scale.y));
  ctx->style.button.text_alignment = NK_TEXT_CENTERED;

  if (nk_begin(
        ctx, "settings", nk_rect(0, space * scale.y, static_cast<float>(size.x), static_cast<float>(size.y)),
        NK_WINDOW_BACKGROUND)) {
    static const std::array<float, 3> ratio{ 0.f, 1.f, 0.f };
    static const std::array<float, 3> button_ratio{ 0.f, 0.25f, 0.f };

    nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, button_ratio.data());

    nk_spacing(ctx, 1);

    if (nk_button_label(ctx, "SAVE")) {
      settings.serialize();
      game.setSettingsChanged(true);
      game.setState(GameState::MENU);
    }
    nk_spacing(ctx, 1);

    nk_layout_row_dynamic(ctx, setting_height * scale.y, 2);
    nk_label(ctx, "Resolution:", NK_TEXT_LEFT);
    if (nk_combo_begin_label(
          ctx, res_to_string(supported_resolutions[selected_res]).c_str(), nk_vec2(nk_widget_width(ctx), 200))) {
      nk_layout_row_dynamic(ctx, setting_height * scale.y, 1);
      for (size_t i = 0; i < supported_resolutions.size(); i++) {
        auto res = supported_resolutions[i];
        if (static_cast<int>(video_mode.width) >= res.first && static_cast<int>(video_mode.height) >= res.second) {
          if (nk_combo_item_label(ctx, res_to_string(res).c_str(), NK_TEXT_LEFT)) {
            selected_res = i;
            settings.set_resolution(res);
          }
        }
      }
      nk_combo_end(ctx);
    }

    nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, button_ratio.data());
    nk_spacing(ctx, 1);

    nk_label(ctx, "V-Sync:", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, setting_height * scale.y, 2);
    if (nk_option_label(ctx, "Enabled", settings.vsync())) {
      settings.set_vsync(true);
    }
    if (nk_option_label(ctx, "Disabled", !settings.vsync())) {
      settings.set_vsync(false);
    }

    nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, ratio.data());

    nk_spacing(ctx, 1);

    nk_label(ctx, "Video Mode:", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, setting_height * scale.y, 2);
    if (nk_option_label(ctx, "Fullscreen", settings.fullscreen())) {
      settings.set_fullscreen(true);
    }
    if (nk_option_label(ctx, "Windowed", !settings.fullscreen())) {
      settings.set_fullscreen(false);
    }

    nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, ratio.data());

    nk_spacing(ctx, 1);

    nk_layout_row_begin(ctx, NK_DYNAMIC, setting_height * scale.y, 2);
    {
      nk_layout_row_push(ctx, 0.3f);
      nk_label(ctx, "Volume:", NK_TEXT_LEFT);
      nk_layout_row_push(ctx, 0.5);
      nk_slider_float(ctx, 0, &settings.volume_level, 1.0f, 0.01f);
    }
    nk_layout_row_end(ctx);
  }

  nk_end(ctx);
  nk_style_pop_vec2(ctx);
  nk_style_pop_color(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  pop_window_state();
}

void Nuklear::render_hud() {
  auto ctx = this->ctx();

  push_window_state();
  const float progressbar_height = 24;
  const float margin = 16;

  struct nk_style& s = ctx->style;
  nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

  if (nk_begin(
        ctx, "hud",
        nk_rect(
          size.x * 0.6f, size.y - (progressbar_height + margin) * scale.y, size.x * 0.4f,
          (progressbar_height + margin) * scale.y),
        NK_WINDOW_BACKGROUND)) {
    nk_size currentHealth = stats().health().get();

    nk_layout_row_static(ctx, progressbar_height * scale.y, size.x * 0.4f - margin * static_cast<float>(scale.x), 1);

    ctx->style.progress.normal = nk_style_item_color(nk_rgba(225, 232, 225, 100));

    if (currentHealth > 40 && currentHealth < 70) {
      ctx->style.progress.cursor_normal = nk_style_item_color(nk_rgb(255, 165, 0));
    } else if (currentHealth >= 70) {
      ctx->style.progress.cursor_normal = nk_style_item_color(nk_rgb(36, 109, 36));
    } else {
      ctx->style.progress.cursor_normal = nk_style_item_color(nk_rgb(255, 0, 0));
    }

    ctx->style.progress.padding = nk_vec2(0, 0);
    ctx->style.progress.border = 1 * scale.x;

    nk_progress(ctx, &currentHealth, 100, NK_FIXED);
  }

  nk_end(ctx);
  nk_style_pop_style_item(ctx);
  pop_window_state();
}

std::pair<json, DialogState>
Nuklear::render_response_dialog(const json& dialog, DialogState dialog_state, const json& init) {
  auto ctx = this->ctx();

  push_window_state();
  struct nk_style& s = ctx->style;
  nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(40, 40, 40, 240)));

  const float dialog_height = size.y * 0.10f;
  const float dialog_height_offset = size.y - dialog_height - size.y * 0.05f;
  const float dialog_width = size.x * 0.8f;
  const float dialog_width_offset = (size.x - dialog_width) / 2;

  nk_style_push_style_item(ctx, &s.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
  nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(255, 232, 225, 100)));
  nk_style_push_style_item(ctx, &s.button.active, nk_style_item_color(nk_rgba(255, 232, 225, 200)));
  ctx->style.button.text_alignment = NK_TEXT_LEFT;
  ctx->style.button.border = 0;
  ctx->style.window.border = 10.0f;

  bool selected = false;
  if (nk_begin(
        ctx, "dialog_response", nk_rect(dialog_width_offset, dialog_height_offset, dialog_width, dialog_height),
        NK_WINDOW_BACKGROUND)) {
    static const std::array<float, 3> ratio{ 0.01f, 0.98f, 0.01f };

    nk_layout_row_static(ctx, dialog_height / 5, 15, 1);
    nk_layout_row(ctx, NK_DYNAMIC, 0, 2, ratio.data());
    nk_spacing(ctx, 1);

    if (dialog.is_string()) {
      if (nk_button_label(ctx, dialog.get<std::string>().c_str()))
        selected = true;
    } else {
      if (nk_button_label(ctx, dialog[stateAsString(dialog_state)].get<std::string>().c_str()))
        selected = true;
    }
  }

  nk_end(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  pop_window_state();

  if (!selected) {
    return std::make_pair(dialog, dialog_state);
  } else {
    if (dialog.is_string()) {
      return std::make_pair(init, !dialog_state);
    } else {
      return std::make_pair(dialog, !dialog_state);
    }
  }
}

std::pair<json, DialogState> Nuklear::render_dialog(const json& lines, DialogState dialog_state) {
  auto ctx = this->ctx();

  push_window_state();
  struct nk_style& s = ctx->style;
  nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(40, 40, 40, 240)));

  const float dialog_height = size.y * lines.size() * 0.06f;
  const float dialog_height_offset = size.y - dialog_height - size.y * 0.05f;
  const float dialog_element_height = size.y * 0.05f;
  const float dialog_width = size.x * 0.8f;
  const float dialog_width_offset = (size.x - dialog_width) / 2;

  nk_style_push_style_item(ctx, &s.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
  nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(255, 232, 225, 100)));
  nk_style_push_style_item(ctx, &s.button.active, nk_style_item_color(nk_rgba(255, 232, 225, 200)));
  ctx->style.button.text_alignment = NK_TEXT_LEFT;
  ctx->style.button.border = 0;
  ctx->style.window.border = 10.0f;

  std::optional<int> response;
  if (nk_begin(
        ctx, "dialog", nk_rect(dialog_width_offset, dialog_height_offset, dialog_width, dialog_height),
        NK_WINDOW_BACKGROUND)) {
    static const std::array<float, 3> ratio{ 0.01f, 0.98f, 0.01f };

    nk_layout_row_static(ctx, dialog_height * 0.05f, 15, 1);
    nk_layout_row(ctx, NK_DYNAMIC, dialog_element_height, 2, ratio.data());

    for (size_t i = 0; i < lines.size(); i++) {
      nk_spacing(ctx, 1);

      if (dialog_state == DialogState::QUESTION) {
        if (nk_button_label(ctx, lines[i][stateAsString(dialog_state)].get<std::string>().c_str()))
          response = i;
      } else {
        dialog_state = !dialog_state;
      }
    }
  }

  nk_end(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  nk_style_pop_style_item(ctx);
  pop_window_state();

  if (response) {
    const auto dialog_type = !dialog_state;
    return std::make_pair(lines[*response][stateAsString(dialog_type)], dialog_type);
  } else
    return std::make_pair(lines, dialog_state);
}

Nuklear::Nuklear(
  sf::Vector2u size_, const Stats& stats, const std::shared_ptr<AssetCache> asset_cache_, sf::RenderWindow* _window):
  asset_cache(asset_cache_),
  size(size_), window(_window), _stats(stats), _ctx(Nuklear::init(_window)) {}

void Nuklear::setSize(sf::Vector2u size) {
  this->size = size;
}

void Nuklear::setScale(sf::Vector2f scale) {
  this->scale = scale;
}

} // namespace tol
