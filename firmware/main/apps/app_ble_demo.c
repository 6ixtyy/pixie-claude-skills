#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "host/ble_hs.h"
#include "host/ble_store.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "os/os_mbuf.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "pixie_app.h"
#include "pixie_runtime.h"

#define BLE_TEXT_MAX (32)

enum {
  PIXIE_CHR_MESSAGE = 1,
  PIXIE_CHR_COLOR = 2,
  PIXIE_CHR_BUTTON = 3
};

typedef enum VisualMode {
  VisualModeStatic = 0,
  VisualModePulse = 1,
  VisualModeRainbow = 2
} VisualMode;

static const char *TAG = "pixie_ble_demo";

static const ble_uuid128_t PIXIE_SERVICE_UUID =
  BLE_UUID128_INIT(0x13, 0x72, 0x6c, 0x10, 0x8c, 0x4d, 0x44, 0xaf,
    0xb3, 0x22, 0x47, 0xd7, 0x40, 0xa2, 0x30, 0x11);
static const ble_uuid128_t PIXIE_MESSAGE_UUID =
  BLE_UUID128_INIT(0x14, 0x72, 0x6c, 0x10, 0x8c, 0x4d, 0x44, 0xaf,
    0xb3, 0x22, 0x47, 0xd7, 0x40, 0xa2, 0x30, 0x11);
static const ble_uuid128_t PIXIE_COLOR_UUID =
  BLE_UUID128_INIT(0x15, 0x72, 0x6c, 0x10, 0x8c, 0x4d, 0x44, 0xaf,
    0xb3, 0x22, 0x47, 0xd7, 0x40, 0xa2, 0x30, 0x11);
static const ble_uuid128_t PIXIE_BUTTON_UUID =
  BLE_UUID128_INIT(0x16, 0x72, 0x6c, 0x10, 0x8c, 0x4d, 0x44, 0xaf,
    0xb3, 0x22, 0x47, 0xd7, 0x40, 0xa2, 0x30, 0x11);

static uint16_t button_value_handle = 0;
static uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint8_t own_addr_type = 0;

static bool notify_enabled = false;
static bool ble_started = false;

static char display_text[BLE_TEXT_MAX + 1] = "Tap iPhone";
static char last_button_text[8] = "None";
static uint8_t bg_r = 32;
static uint8_t bg_g = 0;
static uint8_t bg_b = 18;
static VisualMode visual_mode = VisualModeStatic;
static uint8_t anim_phase = 0;
static TickType_t last_anim_tick = 0;

static portMUX_TYPE state_mux = portMUX_INITIALIZER_UNLOCKED;

static void lock_state(void) {
  taskENTER_CRITICAL(&state_mux);
}

static void unlock_state(void) {
  taskEXIT_CRITICAL(&state_mux);
}

static void sanitize_text(char *text) {
  for (char *p = text; *p; p++) {
    if (!isprint((unsigned char)(*p))) {
      *p = ' ';
    }
  }
}

static bool handle_dex_command(const char *text) {
  if (strcmp(text, "/dex top") == 0) {
    visual_mode = VisualModeStatic;
    strncpy(display_text, "Use: /dex top SYM MCAP", sizeof(display_text));
    display_text[sizeof(display_text) - 1] = '\0';
    return true;
  }

  if (strncmp(text, "/dex top ", 9) == 0) {
    const char *payload = text + 9;
    while (*payload == ' ') { payload++; }
    if (*payload == '\0') {
      visual_mode = VisualModeStatic;
      strncpy(display_text, "Use: /dex top SYM MCAP", sizeof(display_text));
      display_text[sizeof(display_text) - 1] = '\0';
      return true;
    }

    char symbol[12] = {0};
    char mcap[16] = {0};
    int parsed = sscanf(payload, "%11s %15s", symbol, mcap);
    if (parsed < 2) {
      visual_mode = VisualModeStatic;
      strncpy(display_text, "Need: /dex top BTC 2T", sizeof(display_text));
      display_text[sizeof(display_text) - 1] = '\0';
      return true;
    }

    for (char *p = symbol; *p; p++) {
      *p = (char)toupper((unsigned char)*p);
    }

    visual_mode = VisualModeStatic;
    snprintf(display_text, sizeof(display_text), "%s $%s", symbol, mcap);
    return true;
  }

  return false;
}

static bool apply_message_command(const char *text) {
  if (handle_dex_command(text)) {
    return true;
  }

  if (strcmp(text, "/mode static") == 0) {
    visual_mode = VisualModeStatic;
    strncpy(display_text, "Static", sizeof(display_text));
    display_text[sizeof(display_text) - 1] = '\0';
    return true;
  }

  if (strcmp(text, "/mode pulse") == 0) {
    visual_mode = VisualModePulse;
    strncpy(display_text, "Pulse", sizeof(display_text));
    display_text[sizeof(display_text) - 1] = '\0';
    return true;
  }

  if (strcmp(text, "/mode rainbow") == 0) {
    visual_mode = VisualModeRainbow;
    strncpy(display_text, "Rainbow", sizeof(display_text));
    display_text[sizeof(display_text) - 1] = '\0';
    return true;
  }

  if (strcmp(text, "/help") == 0) {
    strncpy(display_text, "static pulse rainbow", sizeof(display_text));
    display_text[sizeof(display_text) - 1] = '\0';
    return true;
  }

  return false;
}

static uint8_t triangle_u8(uint8_t phase) {
  if (phase < 128) { return (uint8_t)(phase * 2); }
  return (uint8_t)((255 - phase) * 2);
}

static void color_wheel(uint8_t position, uint8_t *r, uint8_t *g, uint8_t *b) {
  uint8_t p = (uint8_t)(255 - position);
  if (p < 85) {
    *r = (uint8_t)(255 - p * 3);
    *g = 0;
    *b = (uint8_t)(p * 3);
    return;
  }
  if (p < 170) {
    p = (uint8_t)(p - 85);
    *r = 0;
    *g = (uint8_t)(p * 3);
    *b = (uint8_t)(255 - p * 3);
    return;
  }
  p = (uint8_t)(p - 170);
  *r = (uint8_t)(p * 3);
  *g = (uint8_t)(255 - p * 3);
  *b = 0;
}

static void start_advertising(void);

static int gap_event(struct ble_gap_event *event, void *arg) {
  (void)arg;

  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
      if (event->connect.status == 0) {
        lock_state();
        conn_handle = event->connect.conn_handle;
        notify_enabled = false;
        unlock_state();
      } else {
        start_advertising();
      }
      return 0;

    case BLE_GAP_EVENT_DISCONNECT:
      lock_state();
      conn_handle = BLE_HS_CONN_HANDLE_NONE;
      notify_enabled = false;
      unlock_state();
      start_advertising();
      return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
      if (event->subscribe.attr_handle == button_value_handle) {
        lock_state();
        notify_enabled = event->subscribe.cur_notify;
        unlock_state();
      }
      return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
      start_advertising();
      return 0;

    default:
      return 0;
  }
}

static void start_advertising(void) {
  struct ble_hs_adv_fields fields;
  memset(&fields, 0, sizeof(fields));

  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
  fields.tx_pwr_lvl_is_present = 1;

  const char *name = ble_svc_gap_device_name();
  fields.name = (uint8_t *)name;
  fields.name_len = (uint8_t)strlen(name);
  fields.name_is_complete = 1;

  int rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_gap_adv_set_fields rc=%d", rc);
    return;
  }

  struct ble_gap_adv_params adv_params;
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_gap_adv_start rc=%d", rc);
  }
}

static void on_sync(void) {
  int rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_hs_id_infer_auto rc=%d", rc);
    return;
  }
  start_advertising();
}

static void on_reset(int reason) {
  ESP_LOGW(TAG, "BLE reset reason=%d", reason);
}

static int gatt_access(uint16_t conn_handle_arg, uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt, void *arg) {
  (void)conn_handle_arg;
  (void)attr_handle;

  int which = (int)(uintptr_t)arg;

  if (which == PIXIE_CHR_MESSAGE) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      size_t len = OS_MBUF_PKTLEN(ctxt->om);
      if (len > BLE_TEXT_MAX) { len = BLE_TEXT_MAX; }

      char temp[BLE_TEXT_MAX + 1];
      int rc = os_mbuf_copydata(ctxt->om, 0, len, temp);
      if (rc != 0) { return BLE_ATT_ERR_UNLIKELY; }
      temp[len] = '\0';
      sanitize_text(temp);

      lock_state();
      if (!apply_message_command(temp)) {
        visual_mode = VisualModeStatic;
        memcpy(display_text, temp, len + 1);
      }
      unlock_state();
      return 0;
    }

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      char temp[BLE_TEXT_MAX + 1];
      lock_state();
      strncpy(temp, display_text, sizeof(temp));
      temp[sizeof(temp) - 1] = '\0';
      unlock_state();
      return os_mbuf_append(ctxt->om, temp, strlen(temp)) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return BLE_ATT_ERR_UNLIKELY;
  }

  if (which == PIXIE_CHR_COLOR) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      if (OS_MBUF_PKTLEN(ctxt->om) < 3) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
      }

      uint8_t rgb[3];
      int rc = os_mbuf_copydata(ctxt->om, 0, 3, rgb);
      if (rc != 0) { return BLE_ATT_ERR_UNLIKELY; }

      lock_state();
      bg_r = rgb[0];
      bg_g = rgb[1];
      bg_b = rgb[2];
      unlock_state();
      return 0;
    }

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      uint8_t rgb[3];
      lock_state();
      rgb[0] = bg_r;
      rgb[1] = bg_g;
      rgb[2] = bg_b;
      unlock_state();
      return os_mbuf_append(ctxt->om, rgb, sizeof(rgb)) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return BLE_ATT_ERR_UNLIKELY;
  }

  if (which == PIXIE_CHR_BUTTON) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      char temp[sizeof(last_button_text)];
      lock_state();
      strncpy(temp, last_button_text, sizeof(temp));
      temp[sizeof(temp) - 1] = '\0';
      unlock_state();
      return os_mbuf_append(ctxt->om, temp, strlen(temp)) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

    return BLE_ATT_ERR_UNLIKELY;
  }

  return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def gatt_svcs[] = {
  {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = &PIXIE_SERVICE_UUID.u,
    .characteristics = (struct ble_gatt_chr_def[]) {
      {
        .uuid = &PIXIE_MESSAGE_UUID.u,
        .access_cb = gatt_access,
        .arg = (void *)(uintptr_t)PIXIE_CHR_MESSAGE,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP
      },
      {
        .uuid = &PIXIE_COLOR_UUID.u,
        .access_cb = gatt_access,
        .arg = (void *)(uintptr_t)PIXIE_CHR_COLOR,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP
      },
      {
        .uuid = &PIXIE_BUTTON_UUID.u,
        .access_cb = gatt_access,
        .arg = (void *)(uintptr_t)PIXIE_CHR_BUTTON,
        .val_handle = &button_value_handle,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY
      },
      { 0 }
    }
  },
  { 0 }
};

static void ble_host_task(void *arg) {
  (void)arg;
  nimble_port_run();
  nimble_port_freertos_deinit();
}

static void ble_notify_button_change(void) {
  char payload[sizeof(last_button_text)];
  uint16_t handle;
  bool can_notify;

  lock_state();
  handle = conn_handle;
  can_notify = notify_enabled && (handle != BLE_HS_CONN_HANDLE_NONE);
  strncpy(payload, last_button_text, sizeof(payload));
  payload[sizeof(payload) - 1] = '\0';
  unlock_state();

  if (!can_notify) { return; }

  struct os_mbuf *om = ble_hs_mbuf_from_flat(payload, strlen(payload));
  if (om == NULL) { return; }

  int rc = ble_gatts_notify_custom(handle, button_value_handle, om);
  if (rc != 0) {
    ESP_LOGW(TAG, "notify rc=%d", rc);
  }
}

static void ble_start(void) {
  if (ble_started) { return; }

  esp_err_t status = nvs_flash_init();
  if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    status = nvs_flash_init();
  }
  ESP_ERROR_CHECK(status);

  int rc = nimble_port_init();
  if (rc != ESP_OK) {
    ESP_LOGE(TAG, "nimble_port_init rc=%d", rc);
    return;
  }

  ble_hs_cfg.reset_cb = on_reset;
  ble_hs_cfg.sync_cb = on_sync;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

  ble_svc_gap_init();
  ble_svc_gatt_init();
  assert(ble_gatts_count_cfg(gatt_svcs) == 0);
  assert(ble_gatts_add_svcs(gatt_svcs) == 0);
  assert(ble_svc_gap_device_name_set("Pixie BLE Test") == 0);

  nimble_port_freertos_init(ble_host_task);
  ble_started = true;
}

static int find_edge_button(const PixieButtons *buttons) {
  for (int i = 0; i < PIXIE_BUTTON_COUNT; i++) {
    if (pixie_runtime_button_edge(buttons, i)) { return i; }
  }
  return -1;
}

static const char *button_name_from_index(int index) {
  static const char *names[PIXIE_BUTTON_COUNT] = { "A", "B", "C", "D" };
  if (index < 0 || index >= PIXIE_BUTTON_COUNT) { return "None"; }
  return names[index];
}

static void split_message_lines(const char *message, char *line1, size_t line1_size,
    char *line2, size_t line2_size) {
  const size_t first_max = 16;
  size_t msg_len = strlen(message);

  size_t first_len = msg_len;
  if (first_len > first_max) { first_len = first_max; }
  if (first_len >= line1_size) { first_len = line1_size - 1; }

  memcpy(line1, message, first_len);
  line1[first_len] = '\0';

  if (msg_len <= first_max) {
    line2[0] = '\0';
    return;
  }

  size_t second_len = msg_len - first_max;
  if (second_len >= line2_size) { second_len = line2_size - 1; }
  memcpy(line2, message + first_max, second_len);
  line2[second_len] = '\0';
}

static void app_ble_demo_init(void) {
  lock_state();
  strncpy(display_text, "Tap iPhone", sizeof(display_text));
  display_text[sizeof(display_text) - 1] = '\0';
  strncpy(last_button_text, "None", sizeof(last_button_text));
  last_button_text[sizeof(last_button_text) - 1] = '\0';
  bg_r = 32;
  bg_g = 0;
  bg_b = 18;
  visual_mode = VisualModeStatic;
  anim_phase = 0;
  last_anim_tick = 0;
  unlock_state();

  ble_start();
}

static void app_ble_demo_tick(const PixieAppCtx *ctx) {
  lock_state();
  if ((ctx->now - last_anim_tick) >= pdMS_TO_TICKS(33)) {
    last_anim_tick = ctx->now;
    anim_phase++;
  }
  unlock_state();

  int edge = find_edge_button(&ctx->buttons);
  if (edge < 0) { return; }

  const char *label = button_name_from_index(edge);
  lock_state();
  strncpy(last_button_text, label, sizeof(last_button_text));
  last_button_text[sizeof(last_button_text) - 1] = '\0';
  unlock_state();

  ble_notify_button_change();
}

static void app_ble_demo_render(uint8_t *buffer, uint32_t y0, const PixieAppCtx *ctx) {
  (void)ctx;

  uint16_t *framebuffer = (uint16_t *)buffer;
  char message[BLE_TEXT_MAX + 1];
  uint8_t r;
  uint8_t g;
  uint8_t b;
  bool connected;
  VisualMode mode;
  uint8_t phase;

  lock_state();
  strncpy(message, display_text, sizeof(message));
  message[sizeof(message) - 1] = '\0';
  r = bg_r;
  g = bg_g;
  b = bg_b;
  connected = conn_handle != BLE_HS_CONN_HANDLE_NONE;
  mode = visual_mode;
  phase = anim_phase;
  unlock_state();

  char line1[17];
  char line2[17];
  split_message_lines(message, line1, sizeof(line1), line2, sizeof(line2));

  size_t len1 = strlen(line1);
  size_t len2 = strlen(line2);

  int scale = 2;
  if (len2 == 0) {
    if (len1 <= 10) {
      scale = 4;
    } else if (len1 <= 14) {
      scale = 3;
    } else {
      scale = 2;
    }
  } else {
    scale = 2;
  }

  int line_height = 7 * scale;
  int line_gap = scale * 3;
  int total_height = line_height + ((len2 > 0) ? (line_gap + line_height) : 0);
  int y_start = (PIXIE_DISPLAY_HEIGHT - total_height) / 2;

  uint8_t draw_r = r;
  uint8_t draw_g = g;
  uint8_t draw_b = b;

  if (mode == VisualModePulse) {
    uint16_t strength = (uint16_t)(80 + ((uint16_t)triangle_u8(phase) * 175) / 255);
    draw_r = (uint8_t)(((uint16_t)r * strength) / 255);
    draw_g = (uint8_t)(((uint16_t)g * strength) / 255);
    draw_b = (uint8_t)(((uint16_t)b * strength) / 255);
  } else if (mode == VisualModeRainbow) {
    color_wheel(phase, &draw_r, &draw_g, &draw_b);
  }

  uint16_t bg = pixie_runtime_rgb565(draw_r, draw_g, draw_b);
  uint16_t primary = pixie_runtime_rgb565(255, 220, 235);
  uint16_t secondary = pixie_runtime_rgb565(255, 160, 210);
  uint16_t status = connected
    ? pixie_runtime_rgb565(80, 255, 120)
    : pixie_runtime_rgb565(255, 80, 80);

  pixie_runtime_fill_fragment(framebuffer, bg);

  int line1_width = (int)len1 * (6 * scale) - ((len1 > 0) ? scale : 0);
  int x1 = (PIXIE_DISPLAY_WIDTH - line1_width) / 2;
  pixie_runtime_draw_text_fragment(framebuffer, y0, x1, y_start, line1, scale, primary);

  if (len2 > 0) {
    int line2_width = (int)len2 * (6 * scale) - scale;
    int x2 = (PIXIE_DISPLAY_WIDTH - line2_width) / 2;
    pixie_runtime_draw_text_fragment(framebuffer, y0, x2, y_start + line_height + line_gap, line2, scale, secondary);
  }

  pixie_runtime_draw_rect_fragment(framebuffer, y0, 224, 8, 8, 8, status);
}

static void app_ble_demo_deinit(void) {
}

const PixieApp PIXIE_APP_BLE_DEMO = {
  .id = "ble_demo",
  .init = app_ble_demo_init,
  .tick = app_ble_demo_tick,
  .render_fragment = app_ble_demo_render,
  .deinit = app_ble_demo_deinit
};

const PixieApp PIXIE_SELECTED_APP = {
  .id = "ble_demo",
  .init = app_ble_demo_init,
  .tick = app_ble_demo_tick,
  .render_fragment = app_ble_demo_render,
  .deinit = app_ble_demo_deinit
};
