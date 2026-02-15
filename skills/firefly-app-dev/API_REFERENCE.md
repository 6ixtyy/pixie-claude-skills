# Pixie Runtime API Reference

Complete reference for `pixie_runtime.h` and `pixie_app.h`.

---

## pixie_app.h — Constants

```c
PIXIE_DISPLAY_WIDTH          240       // Display width in pixels
PIXIE_DISPLAY_HEIGHT         240       // Display height in pixels
PIXIE_PIN_DISPLAY_DC         4         // Display data/command GPIO
PIXIE_PIN_DISPLAY_RESET      5         // Display reset GPIO
PIXIE_BUTTON_COUNT           4         // Number of buttons
PIXIE_PIN_BUTTON_1           10        // Button 1 GPIO
PIXIE_PIN_BUTTON_2           8         // Button 2 GPIO
PIXIE_PIN_BUTTON_3           3         // Button 3 GPIO
PIXIE_PIN_BUTTON_4           2         // Button 4 GPIO
PIXIE_BUTTON_INDEX_1         0         // Button 1 index
PIXIE_BUTTON_INDEX_2         1         // Button 2 index
PIXIE_BUTTON_INDEX_3         2         // Button 3 index
PIXIE_BUTTON_INDEX_4         3         // Button 4 index
```

---

## pixie_runtime.h — Functions

### pixie_runtime_rgb565

```c
uint16_t pixie_runtime_rgb565(uint8_t red, uint8_t green, uint8_t blue);
```

Convert 8-bit RGB to 16-bit RGB565 format used by the ST7789 display.

**Example**:
```c
uint16_t red = pixie_runtime_rgb565(255, 0, 0);
uint16_t white = pixie_runtime_rgb565(255, 255, 255);
uint16_t dark_blue = pixie_runtime_rgb565(0, 0, 40);
```

---

### pixie_runtime_fill_fragment

```c
void pixie_runtime_fill_fragment(uint16_t *framebuffer, uint16_t color);
```

Fill the entire 240x24 fragment with a solid color. Call this first in `render_fragment()` to clear the background.

---

### pixie_runtime_draw_rect_fragment

```c
void pixie_runtime_draw_rect_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, int width, int height, uint16_t color);
```

Draw a filled rectangle. Automatically clips to the current fragment boundaries.

**Parameters**:
- `x, y`: Top-left corner in screen coordinates (0-239)
- `width, height`: Rectangle dimensions in pixels
- `y0`: Fragment's top Y (passed into render_fragment)

---

### pixie_runtime_draw_char_fragment

```c
void pixie_runtime_draw_char_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, char ch, int scale, uint16_t color);
```

Draw a single character from the built-in 5x7 bitmap font.

**Parameters**:
- `ch`: ASCII character to draw
- `scale`: Pixel multiplier (1 = tiny, 2 = small, 3 = medium, 4 = large)
- Rendered size: `5*scale` wide, `7*scale` tall

---

### pixie_runtime_draw_text_fragment

```c
void pixie_runtime_draw_text_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, const char *text, int scale, uint16_t color);
```

Draw a text string. Characters are spaced `6*scale` pixels apart.

**Centering formula**:
```c
int text_width = strlen(text) * 6 * scale - scale;
int x = (PIXIE_DISPLAY_WIDTH - text_width) / 2;
```

---

### pixie_runtime_buttons_init

```c
void pixie_runtime_buttons_init(void);
```

Initialize button GPIOs with pull-up resistors. Called once by `main.c` at boot -- apps don't need to call this.

---

### pixie_runtime_poll_buttons

```c
PixieButtons pixie_runtime_poll_buttons(void);
```

Sample all 4 buttons with 25ms debounce. Called once per frame by `main.c` -- apps read the result via `ctx->buttons`.

---

### pixie_runtime_button_pressed

```c
bool pixie_runtime_button_pressed(const PixieButtons *buttons, uint8_t index);
```

Check if a button is currently held down.

---

### pixie_runtime_button_edge

```c
bool pixie_runtime_button_edge(const PixieButtons *buttons, uint8_t index);
```

Check if a button was just pressed this frame (rising edge). Use this for single-press actions.

---

### pixie_runtime_now

```c
TickType_t pixie_runtime_now(void);
```

Get current FreeRTOS tick count. Use for timing calculations.

---

### pixie_runtime_elapsed_ms

```c
uint32_t pixie_runtime_elapsed_ms(TickType_t now, TickType_t since);
```

Calculate elapsed milliseconds between two tick values.

---

## Display Host Loop (main.c)

The host loop in `main.c` runs this cycle:
1. `pixie_runtime_poll_buttons()` → snapshot button state
2. `PIXIE_SELECTED_APP.tick(ctx)` → update app logic
3. `ffx_display_renderFragment(display)` → renders one 240x24 strip
4. After all 10 strips: `vTaskDelay(16ms)` → ~60fps target

Apps never call the display driver directly. The host calls `render_fragment()` 10 times per frame with different `y0` values.
