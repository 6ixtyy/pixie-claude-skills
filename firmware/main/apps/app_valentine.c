#include "pixie_app.h"
#include "pixie_runtime.h"

#define HEART_TOGGLE_MS (350)

static TickType_t last_pulse_tick;
static uint8_t theme_index;
static bool pulse_on;

static uint16_t bg_dark;
static uint16_t bg_glow;
static uint16_t text_light;
static uint16_t text_soft;
static uint16_t heart_a;
static uint16_t heart_b;

static void draw_heart(uint16_t *framebuffer, uint32_t y0, int x, int y, int scale, uint16_t color) {
  static const uint8_t shape[5][6] = {
    {0, 1, 1, 0, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 0, 0}
  };

  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 6; col++) {
      if (shape[row][col] == 0) { continue; }
      pixie_runtime_draw_rect_fragment(framebuffer, y0,
        x + (col * scale), y + (row * scale), scale, scale, color);
    }
  }
}

static void update_theme_colors(void) {
  if (theme_index == 0) {
    bg_dark = pixie_runtime_rgb565(32, 0, 16);
    bg_glow = pixie_runtime_rgb565(56, 0, 22);
    text_light = pixie_runtime_rgb565(255, 210, 235);
    text_soft = pixie_runtime_rgb565(255, 150, 200);
    heart_a = pixie_runtime_rgb565(255, 60, 120);
    heart_b = pixie_runtime_rgb565(255, 130, 190);
    return;
  }

  if (theme_index == 1) {
    bg_dark = pixie_runtime_rgb565(26, 0, 8);
    bg_glow = pixie_runtime_rgb565(48, 0, 14);
    text_light = pixie_runtime_rgb565(255, 235, 210);
    text_soft = pixie_runtime_rgb565(255, 190, 170);
    heart_a = pixie_runtime_rgb565(255, 80, 90);
    heart_b = pixie_runtime_rgb565(255, 155, 100);
    return;
  }

  bg_dark = pixie_runtime_rgb565(18, 0, 20);
  bg_glow = pixie_runtime_rgb565(36, 0, 40);
  text_light = pixie_runtime_rgb565(245, 220, 255);
  text_soft = pixie_runtime_rgb565(210, 150, 255);
  heart_a = pixie_runtime_rgb565(235, 95, 255);
  heart_b = pixie_runtime_rgb565(255, 170, 240);
}

static void app_valentine_init(void) {
  theme_index = 0;
  pulse_on = false;
  last_pulse_tick = 0;
  update_theme_colors();
}

static void app_valentine_tick(const PixieAppCtx *ctx) {
  if (ctx->buttons.edge_mask != 0) {
    theme_index = (uint8_t)((theme_index + 1) % 3);
    update_theme_colors();
  }

  if ((ctx->now - last_pulse_tick) >= pdMS_TO_TICKS(HEART_TOGGLE_MS)) {
    last_pulse_tick = ctx->now;
    pulse_on = !pulse_on;
  }
}

static void app_valentine_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  (void)ctx;
  uint16_t *framebuffer = (uint16_t *)buffer;
  uint16_t bg = pulse_on ? bg_glow : bg_dark;
  uint16_t heart_color = pulse_on ? heart_b : heart_a;

  pixie_runtime_fill_fragment(framebuffer, bg);

  pixie_runtime_draw_rect_fragment(framebuffer, y0, 0, 24, PIXIE_DISPLAY_WIDTH, 4, heart_color);
  pixie_runtime_draw_rect_fragment(framebuffer, y0, 0, 212, PIXIE_DISPLAY_WIDTH, 4, heart_color);

  draw_heart(framebuffer, y0, 22, 40, 5, heart_color);
  draw_heart(framebuffer, y0, 174, 40, 5, heart_color);
  draw_heart(framebuffer, y0, 98, 168, 6, heart_color);

  pixie_runtime_draw_text_fragment(framebuffer, y0, 30, 88, "hey Princess", 3, text_light);
  pixie_runtime_draw_text_fragment(framebuffer, y0, 70, 134, "Zaam", 3, text_soft);
}

static void app_valentine_deinit(void) {
}

const PixieApp PIXIE_APP_VALENTINE = {
  .id = "valentine",
  .init = app_valentine_init,
  .tick = app_valentine_tick,
  .render_fragment = app_valentine_render,
  .deinit = app_valentine_deinit
};

const PixieApp PIXIE_SELECTED_APP = {
  .id = "valentine",
  .init = app_valentine_init,
  .tick = app_valentine_tick,
  .render_fragment = app_valentine_render,
  .deinit = app_valentine_deinit
};
