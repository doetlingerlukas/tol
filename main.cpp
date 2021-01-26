#include <iostream>

#include <SFML/Graphics.hpp>

#include <map.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SFML_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sfml_gl3.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

const int window_width = 840;
const int window_height = 600;

struct nk_context* init_nuklear(sf::RenderWindow* window) {
  struct nk_context *ctx;
  ctx = nk_sfml_init(window);

  struct nk_font_atlas *atlas;
  nk_sfml_font_stash_begin(&atlas);
  nk_sfml_font_stash_end();
  return ctx;
}

void render_nuklear_menu(struct nk_context* ctx) {
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

int main() {
  sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Tales of Lostness", sf::Style::Titlebar | sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);

  if(!gladLoadGL()) { /* Load OpenGL extensions */
    printf("Failed to load OpenGL extensions!\n");
    return -1;
  }

  glViewport(0, 0, window.getSize().x, window.getSize().y);
  TiledMap map(std::string("assets/ultimate-map.json"));

  auto ctx = init_nuklear(&window);

  while (window.isOpen()) {
    sf::Event event;
    nk_input_begin(ctx);
    while (window.pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed:
          nk_sfml_shutdown();
          window.close();
          break;
        case sf::Event::KeyPressed:
          if (event.key.code == sf::Keyboard::Escape) {
            nk_sfml_shutdown();
            window.close();
          }
          break;
      }

      nk_sfml_handle_event(&event);
    }

    nk_input_end(ctx);

    render_nuklear_menu(ctx);

    window.setActive(true);
    window.draw(map);
    /* IMPORTANT: `nk_sfml_render` modifies some global OpenGL state
    * with blending, scissor, face culling and depth test and defaults everything
    * back into a default state. Make sure to either save and restore or
    * reset your own state after drawing rendering the UI. */
    nk_sfml_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);

    window.display();
  }

  nk_sfml_shutdown();
  return 0;
}
