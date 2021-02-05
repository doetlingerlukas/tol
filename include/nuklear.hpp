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
#include "nuklear.h"
#include "nuklear_sfml_gl2.h"

#include <SFML/Graphics.hpp>

#include <asset_cache.hpp>
#include <stats.hpp>

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

  void renderMenu() const {
    const float button_height = 40;
    const int r = 0;
    const int g = 0;
    const int b = 0;
    const int a = 0;

    struct nk_color background = nk_rgba(r, g, b, a);

    const auto* font = asset_cache->loadNkFont("fonts/Gaegu-Regular.ttf", 32 * scale.y);
    nk_style_set_font(ctx, &font->handle);

    struct nk_style& s = ctx->style;

    const float spacing = 4;

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
      static const float ratio[] = {0.05f, 0.35f, 0.6f};

      nk_layout_row_static(ctx, (size.y - (button_height * 4.f + spacing * 5.f) * scale.y) / 2.f, 15, 1);
      nk_layout_row(ctx, NK_DYNAMIC, button_height * scale.y, 2, ratio);

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "CONTINUE"))
        fprintf(stdout, "continue pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "LOAD"))
        fprintf(stdout, "load pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "SETTINGS"))
        fprintf(stdout, "settings pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "EXIT"))
        fprintf(stdout, "exit pressed\n");
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

  void renderDialog() {
    struct nk_style& s = ctx->style;
    nk_style_push_style_item(ctx, &s.window.fixed_background, nk_style_item_color(nk_rgba(40, 40, 40, 240)));

    const float dialog_height = size.y * 0.25;
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

    if (nk_begin(ctx, "dialog", nk_rect(dialog_width_offset, dialog_height_offset, dialog_width, dialog_height), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = {0.01f, 0.9f, 0.09f};

      nk_layout_row_static(ctx, dialog_height * 0.065, 15, 1);
      nk_layout_row(ctx, NK_DYNAMIC, dialog_element_height, 2, ratio);

      nk_spacing(ctx, 1);
      if (nk_button_label(ctx, "1) Option 1"))
        fprintf(stdout, "option 1 pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "2) Option 2"))
        fprintf(stdout, "option 2 pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "3) Option 3"))
        fprintf(stdout, "option 3 pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "4) Option 4"))
        fprintf(stdout, "option 4 pressed\n");
    }

    nk_end(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
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

