#ifndef __PIXIE_APP_H__
#define __PIXIE_APP_H__

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"

#define PIXIE_DISPLAY_WIDTH          (240)
#define PIXIE_DISPLAY_HEIGHT         (240)

#define PIXIE_PIN_DISPLAY_DC         (4)
#define PIXIE_PIN_DISPLAY_RESET      (5)

#define PIXIE_BUTTON_COUNT           (4)
#define PIXIE_PIN_BUTTON_1           (10)
#define PIXIE_PIN_BUTTON_2           (8)
#define PIXIE_PIN_BUTTON_3           (3)
#define PIXIE_PIN_BUTTON_4           (2)

#define PIXIE_BUTTON_INDEX_1         (0)
#define PIXIE_BUTTON_INDEX_2         (1)
#define PIXIE_BUTTON_INDEX_3         (2)
#define PIXIE_BUTTON_INDEX_4         (3)

typedef struct PixieButtons {
  uint8_t pressed_mask;
  uint8_t edge_mask;
} PixieButtons;

typedef struct PixieAppCtx {
  TickType_t now;
  PixieButtons buttons;
} PixieAppCtx;

typedef struct PixieApp {
  const char *id;
  void (*init)(void);
  void (*tick)(const PixieAppCtx *ctx);
  void (*render_fragment)(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx);
  void (*deinit)(void);
} PixieApp;

#endif /* __PIXIE_APP_H__ */
