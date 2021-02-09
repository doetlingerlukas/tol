#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include <cstdarg>
#include <cstring>
#include <optional>
#include "nuklear.h"
#include "nuklear_sfml_gl2.h"

#include <SFML/Graphics.hpp>

#include <asset_cache.hpp>
#include <stats.hpp>
#include <dialog_state.hpp>
#include <nlohmann/json.hpp>
#include <game_state.hpp>
#include <settings.hpp>

using json = nlohmann::json;

class Nuklear {
  std::shared_ptr<AssetCache> asset_cache;
  sf::Vector2u size;
  sf::Vector2f scale;
  const std::shared_ptr<Stats> stats;
  std::chrono::milliseconds current_time;
  struct nk_context* ctx;

  struct nk_context* init(sf::RenderWindow* window) const {
    struct nk_context *ctx;
    ctx = nk_sfml_init(window);
    return ctx;
  }
public:
  struct nk_context* getCtx() const {
    return ctx;
  }

  void renderMenu(GameInstance& game) const {
    const float button_height = 40;
    const int r = 0;
    const int g = 0;
    const int b = 0;
    const int a = 0;

    struct nk_color background = nk_rgba(r, g, b, a);

    const auto* font = asset_cache->loadNkFont("fonts/Gaegu-Regular.ttf", 32 * scale.y);
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

    /* GUI */
    if (nk_begin(ctx, "menu", nk_rect(0, 0, size.x, size.y), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = { 0.3, 0.4, 0.3 };

      nk_layout_row_static(ctx, (size.y - (button_height * 4.f + spacing * 5.f) * scale.y) / 2.f, 0, 1);
      nk_layout_row(ctx, NK_DYNAMIC, button_height * scale.y, 2, ratio);

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "CONTINUE"))
        game.setState(GameState::PLAY);

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "LOAD"))
        fprintf(stdout, "load pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "SETTINGS"))
        game.setState(GameState::SETTINGS);

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "EXIT"))
        game.setState(GameState::QUIT);
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
  }

  void renderSettings(GameInstance& game, Settings& settings) {
    const float setting_height = 40;
    const float space = 10;

    struct nk_style& s = ctx->style;

    const auto* font = asset_cache->loadNkFont("fonts/Gaegu-Regular.ttf", 32 * scale.y);
    nk_style_set_font(ctx, &font->handle);

    nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
    nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(50, 50, 50, 255)));
    nk_style_push_color(ctx, &s.button.text_hover, nk_rgba(255, 255, 255, 255));
    nk_style_push_vec2(ctx, &s.window.spacing, nk_vec2(0, space * scale.y));

    if (nk_begin(ctx, "settings", nk_rect(0, space * scale.y, size.x, size.y), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = { 0.f, 1.f, 0.f };
      static const float button_ratio[] = { 0.f, 0.25f, 0.f };

      nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, button_ratio);

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "SAVE")) {
        game.setSettingsChanged(true);
        game.setState(GameState::MENU);
      }

      nk_spacing(ctx, 1);

      nk_label(ctx, "V-Sync:", NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, setting_height * scale.y, 2);
      if (nk_option_label(ctx, "Enabled", settings.vsync())) settings.set_vsync(true);
      if (nk_option_label(ctx, "Disabled", !settings.vsync())) settings.set_vsync(false);

      nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, ratio);

      nk_spacing(ctx, 1);

      nk_label(ctx, "Video Mode:", NK_TEXT_LEFT);

      nk_layout_row_dynamic(ctx, setting_height * scale.y, 2);
      if (nk_option_label(ctx, "Fullscreen", settings.fullscreen())) settings.set_fullscreen(true);
      if (nk_option_label(ctx, "Windowed", !settings.fullscreen())) settings.set_fullscreen(false);

      nk_layout_row(ctx, NK_DYNAMIC, setting_height * scale.y, 2, ratio);

      nk_spacing(ctx, 1);

      nk_layout_row_begin(ctx, NK_DYNAMIC, setting_height * scale.y, 2);
      {
        nk_layout_row_push(ctx, 0.3);
        nk_label(ctx, "Volume:", NK_TEXT_LEFT);
        nk_layout_row_push(ctx, 0.5);
        nk_slider_float(ctx, 0, &settings.volume_level , 1.0f, 0.01f);
      }
      nk_layout_row_end(ctx);
    }

    nk_end(ctx);
    nk_style_pop_vec2(ctx);
    nk_style_pop_color(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
  }

  void renderHud() {
    const float progressbar_height = 24;
    const float margin = 16;

    struct nk_style& s = ctx->style;
    nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

    if (nk_begin(ctx, "hud", nk_rect(size.x * 0.6, size.y - (progressbar_height + margin) * scale.y, size.x * 0.4, (progressbar_height + margin) * scale.y), NK_WINDOW_BACKGROUND)) {
      nk_size currentHealth = stats->health().get();

      nk_layout_row_static(ctx, progressbar_height * scale.y, size.x * 0.4 - margin * scale.x, 1);

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
  }

  std::pair<json, DialogState> renderResponseDialog(const json& dialog, DialogState dialog_state, const json& init) {
    struct nk_style& s = ctx->style;
    nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(40, 40, 40, 240)));

    const float dialog_height = size.y * 0.10;
    const float dialog_height_offset = size.y - dialog_height - size.y * 0.05;
    const float dialog_element_height = size.y * 0.05;
    const float dialog_width = size.x * 0.8;
    const float dialog_width_offset = (size.x - dialog_width) / 2;

    nk_style_push_style_item(ctx, &s.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
    nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(255, 232, 225, 100)));
    nk_style_push_style_item(ctx, &s.button.active, nk_style_item_color(nk_rgba(255, 232,225, 200)));
    ctx->style.button.text_alignment = NK_TEXT_LEFT;
    ctx->style.button.border = 0;
    ctx->style.window.border = 10.0f;

    bool selected = false;
    if (nk_begin(ctx, "dialog_response", nk_rect(dialog_width_offset, dialog_height_offset, dialog_width, dialog_height), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = {0.01f, 0.98f, 0.01f};

      nk_layout_row_static(ctx, dialog_height / 5, 15, 1);
      nk_layout_row(ctx, NK_DYNAMIC, 0, 2, ratio);
      nk_spacing(ctx, 1);

      if(dialog.is_string()) {
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

    if (!selected) {
      return std::make_pair(dialog, dialog_state);
    } else {
      if(dialog.is_string()) {
        return std::make_pair(init, !dialog_state);
      } else {
        return std::make_pair(dialog, !dialog_state);
      }
    }
  }

  std::pair<json, DialogState> renderDialog(const json& lines, DialogState dialog_state) {
    struct nk_style& s = ctx->style;
    nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(40, 40, 40, 240)));

    const float dialog_height = size.y * (lines.size() + 1) * 0.06;
    const float dialog_height_offset = size.y - dialog_height - size.y * 0.05;
    const float dialog_element_height = size.y * 0.05;
    const float dialog_width = size.x * 0.8;
    const float dialog_width_offset = (size.x - dialog_width) / 2;

    nk_style_push_style_item(ctx, &s.button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
    nk_style_push_style_item(ctx, &s.button.hover, nk_style_item_color(nk_rgba(255, 232, 225, 100)));
    nk_style_push_style_item(ctx, &s.button.active, nk_style_item_color(nk_rgba(255, 232,225, 200)));
    ctx->style.button.text_alignment = NK_TEXT_LEFT;
    ctx->style.button.border = 0;
    ctx->style.window.border = 10.0f;

    std::optional<int> response;
    if (nk_begin(ctx, "dialog", nk_rect(dialog_width_offset, dialog_height_offset, dialog_width, dialog_height), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = {0.01f, 0.98f, 0.01f};

      nk_layout_row_static(ctx, dialog_height * 0.05, 15, 1);
      nk_layout_row(ctx, NK_DYNAMIC, dialog_element_height, 2, ratio);

      for (int i = 0; i < lines.size(); i++) {
        nk_spacing(ctx, 1);

        if (nk_button_label(ctx, lines[i][stateAsString(dialog_state)].get<std::string>().c_str()))
          response = i;
      }

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "leave"))
        response = -1;
    }

    nk_end(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);

    if (response) {
      const auto dialog_type = !dialog_state;
      if (*response != -1) {
        return std::make_pair(lines[*response][stateAsString(dialog_type)], dialog_type);
      } else {
        return std::make_pair(0, dialog_type);
      }
    } else
      return std::make_pair(lines, dialog_state);
  }

  Nuklear(sf::Vector2u size_, const std::shared_ptr<Stats> stats_, const std::shared_ptr<AssetCache> asset_cache_, sf::RenderWindow* window):
    size(size_), stats(stats_), asset_cache(asset_cache_) {
    ctx = init(window);
  }

  void setSize(sf::Vector2u size) {
    this->size = size;
  }

  void setScale(sf::Vector2f scale) {
    this->scale = scale;
  }
};

