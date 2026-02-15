#include "pixie_app.h"
#include "pixie_runtime.h"

static uint16_t color_bg;
static uint16_t color_text;
static uint16_t color_box_base;
static uint16_t color_box_lit;
static uint8_t mode_index;

static void app_template_init(void) {
  mode_index = 0;
  color_bg = pixie_runtime_rgb565(8, 8, 20);
  color_text = pixie_runtime_rgb565(120, 220, 255);
  color_box_base = pixie_runtime_rgb565(26, 26, 40);
  color_box_lit = pixie_runtime_rgb565(255, 180, 80);
}

static void app_template_tick(const PixieAppCtx *ctx) {
  if (ctx->buttons.edge_mask != 0) {
    mode_index = (uint8_t)((mode_index + 1) % 3);
  }
}

static void app_template_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  uint16_t *framebuffer = (uint16_t *)buffer;
  uint16_t mode_color = color_text;
  if (mode_index == 1) { mode_color = pixie_runtime_rgb565(140, 255, 120); }
  if (mode_index == 2) { mode_color = pixie_runtime_rgb565(255, 120, 200); }

  pixie_runtime_fill_fragment(framebuffer, color_bg);
  pixie_runtime_draw_text_fragment(framebuffer, y0, 22, 64, "Template App", 3, mode_color);
  pixie_runtime_draw_text_fragment(framebuffer, y0, 36, 112, "Press Any Btn", 2, mode_color);

  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    int x = 26 + i * 52;
    uint16_t color = pixie_runtime_button_pressed(&ctx->buttons, i) ? color_box_lit : color_box_base;
    pixie_runtime_draw_rect_fragment(framebuffer, y0, x, 176, 32, 32, color);
  }
}

static void app_template_deinit(void) {
}

const PixieApp PIXIE_APP_TEMPLATE = {
  .id = "template",
  .init = app_template_init,
  .tick = app_template_tick,
  .render_fragment = app_template_render,
  .deinit = app_template_deinit
};

const PixieApp PIXIE_SELECTED_APP = {
  .id = "template",
  .init = app_template_init,
  .tick = app_template_tick,
  .render_fragment = app_template_render,
  .deinit = app_template_deinit
};
