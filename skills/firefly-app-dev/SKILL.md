---
name: firefly-app-dev
description: Create new Firefly Pixie apps. Use when writing apps, creating firmware, implementing display graphics, handling buttons, writing C code for ESP32-C3, or developing Pixie programs.
---

# Firefly App Development

Create custom apps for the Firefly Pixie device.

---

## Quick Start: Create a New App

1. Copy the template:
   ```bash
   cp firmware/main/apps/app_template.c firmware/main/apps/app_myapp.c
   ```

2. Edit `firmware/main/CMakeLists.txt` -- add your app:
   ```cmake
   elseif(PIXIE_APP STREQUAL "myapp")
     set(PIXIE_APP_SOURCE "apps/app_myapp.c")
   ```

3. Edit `firmware/pixie.sh` -- add to APPS array:
   ```bash
   APPS=("handshake" "template" "valentine" "ble_demo" "myapp")
   ```

4. Build and flash:
   ```bash
   cd firmware && ./pixie.sh flash myapp
   ```

---

## The App Contract

Every app is a single `.c` file that exports a `PixieApp` struct with 4 lifecycle functions:

```c
#include "pixie_app.h"
#include "pixie_runtime.h"

// 1. Initialize state (called once at boot)
static void my_init(void) { }

// 2. Update logic (called every frame, ~60fps)
static void my_tick(const PixieAppCtx *ctx) { }

// 3. Render one fragment (called 10 times per frame, y0 = 0,24,48,...216)
static void my_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) { }

// 4. Cleanup (called on shutdown)
static void my_deinit(void) { }

const PixieApp PIXIE_APP_MYAPP = {
  .id = "myapp",
  .init = my_init,
  .tick = my_tick,
  .render_fragment = my_render,
  .deinit = my_deinit
};

// This is what the host loop runs
const PixieApp PIXIE_SELECTED_APP = {
  .id = "myapp",
  .init = my_init,
  .tick = my_tick,
  .render_fragment = my_render,
  .deinit = my_deinit
};
```

---

## Key Types

### PixieAppCtx
```c
typedef struct PixieAppCtx {
  TickType_t now;        // Current tick count (use for timing)
  PixieButtons buttons;  // Button state snapshot
} PixieAppCtx;
```

### PixieButtons
```c
typedef struct PixieButtons {
  uint8_t pressed_mask;  // Bitmask: which buttons are currently held
  uint8_t edge_mask;     // Bitmask: which buttons were JUST pressed this frame
} PixieButtons;
```

---

## Fragment Rendering

The 240x240 display is split into 10 horizontal strips of 240x24 pixels each. `render_fragment()` is called once per strip with:

- `buffer`: Pointer to a 240x24 RGB565 framebuffer (cast to `uint16_t*`)
- `y0`: Top Y coordinate of this fragment (0, 24, 48, ..., 216)

This saves RAM -- only one fragment is in memory at a time.

**Critical**: Every drawing function takes `y0` as a parameter. Only draw pixels within the current fragment's Y range (`y0` to `y0 + 24`).

---

## Display Constants

```c
#define PIXIE_DISPLAY_WIDTH   240
#define PIXIE_DISPLAY_HEIGHT  240
// Fragment height: 24 (defined in firefly-display as FfxDisplayFragmentHeight)
```

---

## Runtime API Quick Reference

### Colors
```c
uint16_t pixie_runtime_rgb565(uint8_t r, uint8_t g, uint8_t b);
// Returns RGB565 color value for the display
```

### Drawing
```c
// Fill entire fragment with solid color
void pixie_runtime_fill_fragment(uint16_t *fb, uint16_t color);

// Draw filled rectangle
void pixie_runtime_draw_rect_fragment(uint16_t *fb, uint32_t y0,
    int x, int y, int width, int height, uint16_t color);

// Draw single character (5x7 bitmap, scalable)
void pixie_runtime_draw_char_fragment(uint16_t *fb, uint32_t y0,
    int x, int y, char ch, int scale, uint16_t color);

// Draw text string
void pixie_runtime_draw_text_fragment(uint16_t *fb, uint32_t y0,
    int x, int y, const char *text, int scale, uint16_t color);
```

### Buttons
```c
void pixie_runtime_buttons_init(void);
PixieButtons pixie_runtime_poll_buttons(void);
bool pixie_runtime_button_pressed(const PixieButtons *b, uint8_t index);
bool pixie_runtime_button_edge(const PixieButtons *b, uint8_t index);
```

### Timing
```c
TickType_t pixie_runtime_now(void);
uint32_t pixie_runtime_elapsed_ms(TickType_t now, TickType_t since);
```

---

## Text Rendering Details

- Built-in 5x7 pixel font, supports: A-Z, a-z, 0-9, space, basic punctuation
- Character width at scale S: `6 * S` pixels (5px glyph + 1px gap)
- Character height at scale S: `7 * S` pixels
- To center text: `x = (240 - strlen(text) * 6 * scale) / 2`

---

## Button Indices

```c
#define PIXIE_BUTTON_INDEX_1  0  // Leftmost button (GPIO 10)
#define PIXIE_BUTTON_INDEX_2  1  // (GPIO 8)
#define PIXIE_BUTTON_INDEX_3  2  // (GPIO 3)
#define PIXIE_BUTTON_INDEX_4  3  // Rightmost button (GPIO 2)
```

Use `edge_mask` for press detection (triggers once), `pressed_mask` for held state.
