#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "nuklear.h"
#include "nuklear_sfml_gl2.h"

#include <SFML/Graphics.hpp>
#include <stats.hpp>

class Nuklear {
private:
  int window_width;
  int window_height;
  const std::shared_ptr<Stats> stats;
  std::chrono::milliseconds current_time;

public:
  struct nk_context* init(sf::RenderWindow* window) const {
    struct nk_context *ctx;
    ctx = nk_sfml_init(window);

    struct nk_font_atlas *atlas;
    nk_sfml_font_stash_begin(&atlas);
    nk_sfml_font_stash_end();
    return ctx;
  }

  void render_menu(struct nk_context* ctx) const {
    const int button_height = 40;
    const int r = 0;
    const int g = 0;
    const int b = 0;
    const int a = 0;

    struct nk_color background = nk_rgba(r, g, b, a);

    struct nk_style *s = &ctx->style;
    nk_style_push_color(ctx, &s->window.background, background);
    nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(r, g, b, a)));

    /* GUI */
    if (nk_begin(ctx, "menu", nk_rect(0, 0, window_width, window_height), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = {0.05f, 0.35f, 0.6f};

      nk_layout_row_static(ctx, (window_height - (button_height + 1) * 4) / 2, 15, 1);
      nk_layout_row(ctx, NK_DYNAMIC, button_height, 2, ratio);

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "continue"))
        fprintf(stdout, "continue pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "load"))
        fprintf(stdout, "load pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "settings"))
        fprintf(stdout, "settings pressed\n");

      nk_spacing(ctx, 1);

      if (nk_button_label(ctx, "exit"))
        fprintf(stdout, "exit pressed\n");
    }

    nk_end(ctx);
    nk_style_pop_color(ctx);
    nk_style_pop_style_item(ctx);
  }

  void render_hud(struct nk_context* ctx) {
    const float hud_height = 60;
    const float progressbar_height = hud_height / 2;

    struct nk_style *s = &ctx->style;
    nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

    if (nk_begin(ctx, "hud", nk_rect(0, window_height - hud_height, window_width, hud_height), NK_WINDOW_BACKGROUND)) {
      static const float ratio[] = {0.75f, 0.25f, 0.05f};

      nk_size currentHealth = (*stats).getHealth();

      nk_layout_row_static(ctx, (hud_height - progressbar_height) / 4, 15, 1);
      nk_layout_row(ctx, NK_DYNAMIC, progressbar_height, 2, ratio);
      nk_spacing(ctx, 1);

      ctx->style.progress.normal = nk_style_item_color(nk_rgba(225, 232, 225, 100));

      if (currentHealth > 40 && currentHealth < 70) {
        ctx->style.progress.cursor_normal = nk_style_item_color(nk_rgb(255, 165, 0));
      } else if (currentHealth >= 70) {
        ctx->style.progress.cursor_normal = nk_style_item_color(nk_rgb(36, 109, 36));
      } else {
        ctx->style.progress.cursor_normal = nk_style_item_color(nk_rgb(255, 0, 0));
      }

      ctx->style.progress.padding = nk_vec2(0,0);
      ctx->style.progress.border = 1;

      nk_progress(ctx, &currentHealth, 100, NK_FIXED);
    }

    nk_end(ctx);
    nk_style_pop_style_item(ctx);
  }

  Nuklear(int _width, int _height, const std::shared_ptr<Stats> _stats) : window_width(_width), window_height(_height), stats(_stats) { }
};

