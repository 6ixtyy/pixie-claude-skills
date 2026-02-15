#include <stddef.h>

#include "driver/gpio.h"

#include "firefly-display.h"

#include "pixie_runtime.h"
#include "freertos/task.h"

typedef struct Glyph {
  char c;
  uint8_t col[5];
} Glyph;

static const uint8_t BUTTON_PINS[PIXIE_BUTTON_COUNT] = {
  PIXIE_PIN_BUTTON_1,
  PIXIE_PIN_BUTTON_2,
  PIXIE_PIN_BUTTON_3,
  PIXIE_PIN_BUTTON_4
};

#define BUTTON_DEBOUNCE_MS (25)

static bool previous_pressed[PIXIE_BUTTON_COUNT] = { false, false, false, false };
static bool previous_raw[PIXIE_BUTTON_COUNT] = { false, false, false, false };
static TickType_t raw_changed_at[PIXIE_BUTTON_COUNT] = { 0, 0, 0, 0 };

static const Glyph GLYPHS[] = {
  { ' ', {0x00, 0x00, 0x00, 0x00, 0x00} },
  { 'A', {0x7e, 0x11, 0x11, 0x11, 0x7e} },
  { 'B', {0x7f, 0x49, 0x49, 0x49, 0x36} },
  { 'C', {0x3e, 0x41, 0x41, 0x41, 0x22} },
  { 'D', {0x7f, 0x41, 0x41, 0x22, 0x1c} },
  { 'E', {0x7f, 0x49, 0x49, 0x49, 0x41} },
  { 'H', {0x7f, 0x08, 0x08, 0x08, 0x7f} },
  { 'I', {0x00, 0x41, 0x7f, 0x41, 0x00} },
  { 'L', {0x7f, 0x40, 0x40, 0x40, 0x40} },
  { 'M', {0x7f, 0x02, 0x0c, 0x02, 0x7f} },
  { 'N', {0x7f, 0x02, 0x0c, 0x10, 0x7f} },
  { 'O', {0x3e, 0x41, 0x41, 0x41, 0x3e} },
  { 'P', {0x7f, 0x09, 0x09, 0x09, 0x06} },
  { 'R', {0x7f, 0x09, 0x19, 0x29, 0x46} },
  { 'S', {0x46, 0x49, 0x49, 0x49, 0x31} },
  { 'T', {0x01, 0x01, 0x7f, 0x01, 0x01} },
  { 'U', {0x3f, 0x40, 0x40, 0x40, 0x3f} },
  { 'Y', {0x03, 0x04, 0x78, 0x04, 0x03} },
  { 'Z', {0x61, 0x51, 0x49, 0x45, 0x43} },
  { 'a', {0x20, 0x54, 0x54, 0x54, 0x78} },
  { 'b', {0x7f, 0x48, 0x44, 0x44, 0x38} },
  { 'c', {0x38, 0x44, 0x44, 0x44, 0x20} },
  { 'd', {0x38, 0x44, 0x44, 0x48, 0x7f} },
  { 'e', {0x38, 0x54, 0x54, 0x54, 0x18} },
  { 'h', {0x7f, 0x08, 0x04, 0x04, 0x78} },
  { 'i', {0x00, 0x44, 0x7d, 0x40, 0x00} },
  { 'l', {0x00, 0x41, 0x7f, 0x40, 0x00} },
  { 'm', {0x7c, 0x04, 0x18, 0x04, 0x78} },
  { 'n', {0x7c, 0x08, 0x04, 0x04, 0x78} },
  { 'o', {0x38, 0x44, 0x44, 0x44, 0x38} },
  { 'p', {0x7c, 0x14, 0x14, 0x14, 0x08} },
  { 'r', {0x7c, 0x08, 0x04, 0x04, 0x08} },
  { 's', {0x48, 0x54, 0x54, 0x54, 0x20} },
  { 't', {0x04, 0x3f, 0x44, 0x40, 0x20} },
  { 'y', {0x0c, 0x50, 0x50, 0x50, 0x3c} },
  { 'z', {0x44, 0x64, 0x54, 0x4c, 0x44} },
};

static const uint8_t *glyph_for(char c) {
  for (size_t i = 0; i < sizeof(GLYPHS) / sizeof(GLYPHS[0]); i++) {
    if (GLYPHS[i].c == c) { return GLYPHS[i].col; }
  }
  return GLYPHS[0].col;
}

static bool button_is_pressed(uint8_t index) {
  return gpio_get_level(BUTTON_PINS[index]) == 0;
}

void pixie_runtime_buttons_init(void) {
  uint64_t pin_mask = 0;
  TickType_t now = xTaskGetTickCount();
  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    pin_mask |= ((uint64_t)1 << BUTTON_PINS[i]);
    previous_pressed[i] = false;
    previous_raw[i] = false;
    raw_changed_at[i] = now;
  }

  gpio_config_t io_conf = {
    .pin_bit_mask = pin_mask,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  gpio_config(&io_conf);
}

PixieButtons pixie_runtime_poll_buttons(void) {
  TickType_t now = xTaskGetTickCount();
  PixieButtons buttons = { 0, 0 };
  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    bool raw_pressed = button_is_pressed(i);
    if (raw_pressed != previous_raw[i]) {
      previous_raw[i] = raw_pressed;
      raw_changed_at[i] = now;
    }

    bool debounced_pressed = previous_pressed[i];
    if ((now - raw_changed_at[i]) >= pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
      debounced_pressed = previous_raw[i];
    }

    if (debounced_pressed) {
      buttons.pressed_mask |= (1u << i);
      if (!previous_pressed[i]) {
        buttons.edge_mask |= (1u << i);
      }
    }
    previous_pressed[i] = debounced_pressed;
  }
  return buttons;
}

bool pixie_runtime_button_pressed(const PixieButtons *buttons, uint8_t index) {
  if (index >= PIXIE_BUTTON_COUNT) { return false; }
  return (buttons->pressed_mask & (1u << index)) != 0;
}

bool pixie_runtime_button_edge(const PixieButtons *buttons, uint8_t index) {
  if (index >= PIXIE_BUTTON_COUNT) { return false; }
  return (buttons->edge_mask & (1u << index)) != 0;
}

TickType_t pixie_runtime_now(void) {
  return xTaskGetTickCount();
}

uint32_t pixie_runtime_elapsed_ms(TickType_t now, TickType_t since) {
  return (uint32_t)((now - since) * portTICK_PERIOD_MS);
}

uint16_t pixie_runtime_rgb565(uint8_t red, uint8_t green, uint8_t blue) {
  return (uint16_t)((((red) & 0xf8) << 8) | (((green) & 0xfc) << 3) | ((blue) >> 3));
}

void pixie_runtime_fill_fragment(uint16_t *framebuffer, uint16_t color) {
  for (int i = 0; i < PIXIE_DISPLAY_WIDTH * FfxDisplayFragmentHeight; i++) {
    framebuffer[i] = color;
  }
}

void pixie_runtime_draw_rect_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, int width, int height, uint16_t color) {
  int fragment_top = (int)y0;
  int fragment_bottom = fragment_top + FfxDisplayFragmentHeight;

  int draw_top = y;
  if (draw_top < fragment_top) { draw_top = fragment_top; }

  int draw_bottom = y + height;
  if (draw_bottom > fragment_bottom) { draw_bottom = fragment_bottom; }

  if (draw_top >= draw_bottom) { return; }

  int draw_left = x;
  if (draw_left < 0) { draw_left = 0; }

  int draw_right = x + width;
  if (draw_right > PIXIE_DISPLAY_WIDTH) { draw_right = PIXIE_DISPLAY_WIDTH; }

  if (draw_left >= draw_right) { return; }

  for (int py = draw_top; py < draw_bottom; py++) {
    int row = (py - fragment_top) * PIXIE_DISPLAY_WIDTH;
    for (int px = draw_left; px < draw_right; px++) {
      framebuffer[row + px] = color;
    }
  }
}

void pixie_runtime_draw_char_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, char ch, int scale, uint16_t color) {
  const uint8_t *glyph = glyph_for(ch);
  int y1 = (int)y0 + FfxDisplayFragmentHeight;

  for (int col = 0; col < 5; col++) {
    uint8_t bits = glyph[col];
    for (int row = 0; row < 7; row++) {
      if ((bits & (1u << row)) == 0) { continue; }

      for (int dx = 0; dx < scale; dx++) {
        for (int dy = 0; dy < scale; dy++) {
          int px = x + col * scale + dx;
          int py = y + row * scale + dy;

          if (px < 0 || px >= PIXIE_DISPLAY_WIDTH) { continue; }
          if (py < (int)y0 || py >= y1) { continue; }

          framebuffer[(py - (int)y0) * PIXIE_DISPLAY_WIDTH + px] = color;
        }
      }
    }
  }
}

void pixie_runtime_draw_text_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, const char *text, int scale, uint16_t color) {
  int cursor_x = x;
  for (const char *p = text; *p; p++) {
    pixie_runtime_draw_char_fragment(framebuffer, y0, cursor_x, y, *p, scale, color);
    cursor_x += 6 * scale;
  }
}
