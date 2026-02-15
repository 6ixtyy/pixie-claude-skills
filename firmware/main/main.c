#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "firefly-display.h"

#include "apps/pixie_app.h"
#include "apps/pixie_runtime.h"

extern const PixieApp PIXIE_SELECTED_APP;

static PixieAppCtx app_ctx = { 0 };

static void render_func(uint8_t *buffer, uint32_t y0, void *context) {
  (void)context;
  if (PIXIE_SELECTED_APP.render_fragment) {
    PIXIE_SELECTED_APP.render_fragment(buffer, y0, &app_ctx);
  }
}

void app_main(void) {
  pixie_runtime_buttons_init();
  if (PIXIE_SELECTED_APP.init) {
    PIXIE_SELECTED_APP.init();
  }

  FfxDisplayContext display = ffx_display_init(FfxDisplaySpiBus2_nocs,
    PIXIE_PIN_DISPLAY_DC, PIXIE_PIN_DISPLAY_RESET,
    FfxDisplayRotationRibbonRight, render_func, NULL);

  while (1) {
    app_ctx.now = pixie_runtime_now();
    app_ctx.buttons = pixie_runtime_poll_buttons();

    if (PIXIE_SELECTED_APP.tick) {
      PIXIE_SELECTED_APP.tick(&app_ctx);
    }

    if (ffx_display_renderFragment(display)) {
      vTaskDelay(pdMS_TO_TICKS(16));
    }
  }

  if (PIXIE_SELECTED_APP.deinit) {
    PIXIE_SELECTED_APP.deinit();
  }
}
