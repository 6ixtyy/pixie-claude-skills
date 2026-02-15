# Error Patterns & Fixes

Known errors encountered during Firefly Pixie development, with exact fixes.

---

## Build Errors

### `ble_gap_adv_set_fields rc=4`
- **Context**: BLE advertisement setup
- **Root cause**: Advertisement data payload exceeds 31-byte BLE limit
- **Fix**: Remove 128-bit service UUID from `ble_hs_adv_fields`. Keep only flags + TX power + device name. Service UUID is discoverable after GATT connection.

### `ble_store_config_init` not declared
- **Context**: BLE initialization
- **Root cause**: Symbol/header not available in this build config
- **Fix**: Remove `ble_store_config_init()` call and its include. Keep `nimble_port_init`, `ble_svc_gap_init`, `ble_svc_gatt_init`, and `ble_hs_cfg` callbacks.

### `GPIO_OUT_W1TS_REG` / `GPIO_OUT_W1TC_REG` undefined
- **Context**: `firefly-display/src/display.c` SPI pre-transfer callback
- **Root cause**: Old register macros removed in ESP-IDF 6.x
- **Fix**: Replace with `gpio_ll_set_level(&GPIO, gpio_num, level)`. Already fixed in vendored components.

### Missing `esp_driver_gpio` / `esp_driver_spi`
- **Context**: Component dependencies
- **Root cause**: ESP-IDF 6.x split driver components
- **Fix**: Add to component's `CMakeLists.txt` REQUIRES list. Already added in vendored `firefly-display`.

### kconfig parsing error
- **Context**: `idf.py build` with old sdkconfig
- **Root cause**: sdkconfig from older ESP-IDF version has deprecated keys
- **Fix**: Delete sdkconfig and regenerate:
  ```bash
  idf.py -B build_clean set-target esp32c3 build
  ```

---

## Flash Errors

### `No serial port detected`
- **Fix**: Check USB connection, then: `ls /dev/cu.usb*`
- **Pass explicit port**: `./pixie.sh flash <app> /dev/cu.usbmodem101`

### Docker `--device` passthrough fails
- **Context**: Trying to flash from inside Docker on macOS
- **Root cause**: macOS doesn't support serial device passthrough to Docker
- **Fix**: Always flash from host. `pixie.sh` already does this correctly.

### Flash succeeds but device unchanged
- **Fix**: Power cycle (unplug/replug USB), verify correct app ID, delete `build_clean/` and rebuild

---

## Runtime Errors

### BLE not visible to phone scanner
- **Check**: Serial logs for `ble_gap_adv_set_fields rc=` errors
- **Check**: sdkconfig has `CONFIG_BT_ENABLED=y`, `CONFIG_BT_NIMBLE_ENABLED=y`
- **Check**: NimBLE host stack size >= 4096 in sdkconfig

### Display shows nothing
- **Check**: SPI bus is `FfxDisplaySpiBus2_nocs`
- **Check**: DC pin = 4, Reset pin = 5
- **Check**: `render_fragment` is not returning without drawing

### Buttons not responding
- **Check**: `pixie_runtime_buttons_init()` called (done by main.c)
- **Check**: Using correct GPIO pins (10, 8, 3, 2)
- **Check**: Using `edge_mask` not `pressed_mask` for single-press detection
