#include <string.h>
#include <stdbool.h>

#include "pixie_app.h"
#include "pixie_runtime.h"

#define HANDSHAKE_LEN            (5)
#define INPUT_TIMEOUT_MS         (2500)
#define UNLOCK_HOLD_MS           (8000)
#define ERROR_FLASH_MS           (500)

static const uint8_t HANDSHAKE_PATTERN[HANDSHAKE_LEN] = {
  PIXIE_BUTTON_INDEX_1,
  PIXIE_BUTTON_INDEX_3,
  PIXIE_BUTTON_INDEX_2,
  PIXIE_BUTTON_INDEX_4,
  PIXIE_BUTTON_INDEX_1
};

static uint8_t pattern_progress = 0;
static bool unlocked = false;
static TickType_t last_input_tick = 0;
static TickType_t unlocked_since = 0;
static TickType_t error_until = 0;

static uint16_t color_bg;
static uint16_t color_text;
static uint16_t color_error_bg;
static uint16_t color_unlocked_bg;
static uint16_t color_unlocked_text;
static uint16_t color_dim;
static uint16_t color_button_base;
static uint16_t color_button_locked;
static uint16_t color_button_unlocked;

static int find_edge_button(const PixieButtons *buttons) {
  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    if (pixie_runtime_button_edge(buttons, i)) { return i; }
  }
  return -1;
}

static void app_handshake_init(void) {
  pattern_progress = 0;
  unlocked = false;
  last_input_tick = 0;
  unlocked_since = 0;
  error_until = 0;

  color_bg = pixie_runtime_rgb565(0, 0, 0);
  color_text = pixie_runtime_rgb565(255, 80, 120);
  color_error_bg = pixie_runtime_rgb565(65, 0, 0);
  color_unlocked_bg = pixie_runtime_rgb565(0, 22, 0);
  color_unlocked_text = pixie_runtime_rgb565(80, 255, 120);
  color_dim = pixie_runtime_rgb565(32, 32, 32);
  color_button_base = pixie_runtime_rgb565(28, 28, 28);
  color_button_locked = pixie_runtime_rgb565(160, 80, 220);
  color_button_unlocked = pixie_runtime_rgb565(0, 180, 80);
}

static void app_handshake_tick(const PixieAppCtx *ctx) {
  if (unlocked) {
    if ((ctx->now - unlocked_since) >= pdMS_TO_TICKS(UNLOCK_HOLD_MS)) {
      unlocked = false;
    }
    return;
  }

  if (pattern_progress > 0 &&
      (ctx->now - last_input_tick) >= pdMS_TO_TICKS(INPUT_TIMEOUT_MS)) {
    pattern_progress = 0;
  }

  int pressed_button = find_edge_button(&ctx->buttons);
  if (pressed_button < 0) { return; }

  last_input_tick = ctx->now;
  if (pressed_button == HANDSHAKE_PATTERN[pattern_progress]) {
    pattern_progress++;
    if (pattern_progress == HANDSHAKE_LEN) {
      unlocked = true;
      unlocked_since = ctx->now;
      pattern_progress = 0;
      error_until = 0;
    }
    return;
  }

  pattern_progress = (pressed_button == HANDSHAKE_PATTERN[0]) ? 1 : 0;
  error_until = ctx->now + pdMS_TO_TICKS(ERROR_FLASH_MS);
}

static void app_handshake_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  uint16_t *framebuffer = (uint16_t *)buffer;
  bool flash_error = (!unlocked && ctx->now < error_until);

  uint16_t bg = flash_error ? color_error_bg : (unlocked ? color_unlocked_bg : color_bg);
  uint16_t accent = unlocked ? color_unlocked_text : color_text;
  const char *msg = unlocked ? "Hey Hibinyo" : "Hey Samito";

  pixie_runtime_fill_fragment(framebuffer, bg);

  int scale = 3;
  int char_w = 6 * scale;
  int msg_w = ((int)strlen(msg) * char_w) - scale;
  int x0 = (PIXIE_DISPLAY_WIDTH - msg_w) / 2;

  pixie_runtime_draw_text_fragment(framebuffer, y0, x0, 84, msg, scale, accent);

  int progress_width = (HANDSHAKE_LEN * 14) + ((HANDSHAKE_LEN - 1) * 6);
  int progress_x = (PIXIE_DISPLAY_WIDTH - progress_width) / 2;
  for (int i = 0; i < HANDSHAKE_LEN; i++) {
    uint16_t color = (i < pattern_progress) ? accent : color_dim;
    pixie_runtime_draw_rect_fragment(framebuffer, y0, progress_x + i * 20, 150, 14, 14, color);
  }

  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    int x_button = 26 + i * 52;
    uint16_t lit = unlocked ? color_button_unlocked : color_button_locked;
    uint16_t color = pixie_runtime_button_pressed(&ctx->buttons, i) ? lit : color_button_base;
    pixie_runtime_draw_rect_fragment(framebuffer, y0, x_button, 192, 32, 32, color);
  }
}

static void app_handshake_deinit(void) {
}

const PixieApp PIXIE_APP_HANDSHAKE = {
  .id = "handshake",
  .init = app_handshake_init,
  .tick = app_handshake_tick,
  .render_fragment = app_handshake_render,
  .deinit = app_handshake_deinit
};

const PixieApp PIXIE_SELECTED_APP = {
  .id = "handshake",
  .init = app_handshake_init,
  .tick = app_handshake_tick,
  .render_fragment = app_handshake_render,
  .deinit = app_handshake_deinit
};
