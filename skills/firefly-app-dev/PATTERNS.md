# Common App Patterns

Reusable patterns for Firefly Pixie apps.

---

## Pattern: Animation Loop

Use tick counting for timed animations:

```c
static TickType_t last_frame;
static uint8_t frame_counter;

static void my_tick(const PixieAppCtx *ctx) {
  if ((ctx->now - last_frame) >= pdMS_TO_TICKS(100)) {  // 10fps animation
    last_frame = ctx->now;
    frame_counter++;
  }
}
```

---

## Pattern: Button State Machine

Cycle through modes on any button press:

```c
static uint8_t mode = 0;

static void my_tick(const PixieAppCtx *ctx) {
  if (ctx->buttons.edge_mask != 0) {
    mode = (mode + 1) % 4;
  }
}
```

React to specific buttons:

```c
static void my_tick(const PixieAppCtx *ctx) {
  if (pixie_runtime_button_edge(&ctx->buttons, PIXIE_BUTTON_INDEX_1)) {
    // Button A pressed
  }
  if (pixie_runtime_button_edge(&ctx->buttons, PIXIE_BUTTON_INDEX_4)) {
    // Button D pressed
  }
}
```

---

## Pattern: Centered Text

```c
static void my_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  uint16_t *fb = (uint16_t *)buffer;
  pixie_runtime_fill_fragment(fb, pixie_runtime_rgb565(0, 0, 0));

  const char *msg = "Hello Pixie";
  int scale = 3;
  int text_w = strlen(msg) * 6 * scale - scale;
  int x = (PIXIE_DISPLAY_WIDTH - text_w) / 2;
  int y = (PIXIE_DISPLAY_HEIGHT - 7 * scale) / 2;

  pixie_runtime_draw_text_fragment(fb, y0, x, y, msg, scale,
    pixie_runtime_rgb565(255, 255, 255));
}
```

---

## Pattern: Color Pulse

Smoothly pulse a background color:

```c
static uint8_t phase = 0;

static void my_tick(const PixieAppCtx *ctx) {
  phase++;  // wraps at 255 automatically
}

static void my_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  uint16_t *fb = (uint16_t *)buffer;

  // Triangle wave: 0→255→0
  uint8_t brightness = (phase < 128) ? (phase * 2) : ((255 - phase) * 2);
  uint16_t bg = pixie_runtime_rgb565(brightness / 4, 0, brightness / 2);

  pixie_runtime_fill_fragment(fb, bg);
}
```

---

## Pattern: Button Indicator Row

Show 4 button states at the bottom of the screen:

```c
static void my_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  uint16_t *fb = (uint16_t *)buffer;
  uint16_t off = pixie_runtime_rgb565(28, 28, 28);
  uint16_t on = pixie_runtime_rgb565(255, 180, 80);

  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    uint16_t color = pixie_runtime_button_pressed(&ctx->buttons, i) ? on : off;
    pixie_runtime_draw_rect_fragment(fb, y0, 26 + i * 52, 192, 32, 32, color);
  }
}
```

---

## Pattern: Elapsed Time Display

Show a counter or timer:

```c
static TickType_t start_time;
static char time_str[16];

static void my_init(void) {
  start_time = pixie_runtime_now();
}

static void my_tick(const PixieAppCtx *ctx) {
  uint32_t ms = pixie_runtime_elapsed_ms(ctx->now, start_time);
  uint32_t seconds = ms / 1000;
  snprintf(time_str, sizeof(time_str), "%lu", (unsigned long)seconds);
}
```

---

## Pattern: Multi-Line Text

Split long messages across two lines:

```c
static void render_two_lines(uint16_t *fb, uint32_t y0,
    const char *line1, const char *line2, int scale, uint16_t color) {
  int line_h = 7 * scale;
  int gap = scale * 3;
  int total_h = line_h * 2 + gap;
  int y_start = (PIXIE_DISPLAY_HEIGHT - total_h) / 2;

  int w1 = strlen(line1) * 6 * scale - scale;
  pixie_runtime_draw_text_fragment(fb, y0, (240 - w1) / 2, y_start, line1, scale, color);

  int w2 = strlen(line2) * 6 * scale - scale;
  pixie_runtime_draw_text_fragment(fb, y0, (240 - w2) / 2, y_start + line_h + gap, line2, scale, color);
}
```
