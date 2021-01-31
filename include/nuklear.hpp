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

class Nuklear {
private:
  int window_width;
  int window_height;

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

  Nuklear(int _width, int _height) : window_width(_width), window_height(_height) { }
};

