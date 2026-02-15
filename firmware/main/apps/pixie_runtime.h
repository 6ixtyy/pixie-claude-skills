#ifndef __PIXIE_RUNTIME_H__
#define __PIXIE_RUNTIME_H__

#include <stdint.h>
#include <stdbool.h>

#include "pixie_app.h"

void pixie_runtime_buttons_init(void);
PixieButtons pixie_runtime_poll_buttons(void);
bool pixie_runtime_button_pressed(const PixieButtons *buttons, uint8_t index);
bool pixie_runtime_button_edge(const PixieButtons *buttons, uint8_t index);
TickType_t pixie_runtime_now(void);
uint32_t pixie_runtime_elapsed_ms(TickType_t now, TickType_t since);

uint16_t pixie_runtime_rgb565(uint8_t red, uint8_t green, uint8_t blue);

void pixie_runtime_fill_fragment(uint16_t *framebuffer, uint16_t color);
void pixie_runtime_draw_rect_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, int width, int height, uint16_t color);
void pixie_runtime_draw_char_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, char ch, int scale, uint16_t color);
void pixie_runtime_draw_text_fragment(uint16_t *framebuffer, uint32_t y0,
    int x, int y, const char *text, int scale, uint16_t color);

#endif /* __PIXIE_RUNTIME_H__ */
