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

public:
  struct nk_context* init(sf::RenderWindow* window) const {
    struct nk_context *ctx;
    ctx = nk_sfml_init(window);
    return ctx;
  }

  void render_menu(struct nk_context* ctx) const {
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

  void render_hud(struct nk_context* ctx) {
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

  Nuklear(sf::Vector2u size_, const std::shared_ptr<Stats> stats_, const std::shared_ptr<AssetCache> asset_cache_):
    size(size_), stats(stats_), asset_cache(asset_cache_) {}

  void setSize(sf::Vector2u size) {
    this->size = size;
  }

  void setScale(sf::Vector2f scale) {
    this->scale = scale;
  }
};

