---
name: firefly-debug
description: Debug Firefly Pixie firmware issues. Use when troubleshooting build errors, flash failures, BLE problems, display issues, serial port problems, or ESP-IDF errors on the Pixie device.
---

# Firefly Debug

Diagnose and fix common Firefly Pixie issues.

---

## Debugging Checklist

1. **Is the device connected?** `ls /dev/cu.usb*`
2. **Is Docker running?** `docker info` (needed for builds)
3. **Can you list apps?** `cd firmware && ./pixie.sh list`
4. **Does the build succeed?** `./pixie.sh build template`
5. **Does the flash succeed?** Check for "Hash of data verified" in output
6. **Is the device responding?** Power cycle by unplugging and replugging USB

---

## Serial Log Capture

To read device boot/runtime logs:

```bash
cd firmware
python3 -m venv .venv-serial 2>/dev/null || true
.venv-serial/bin/pip install pyserial -q

# Read serial output
.venv-serial/bin/python -c "
import serial, sys
with serial.Serial('/dev/cu.usbmodem101', 115200, timeout=1) as s:
  while True:
    line = s.readline()
    if line:
      sys.stdout.buffer.write(line)
      sys.stdout.flush()
"
```

Press Ctrl+C to stop. Power-cycle the device to see boot messages.

---

## Common Error Patterns

### BLE scanner can't find device
**Log**: `ble_gap_adv_set_fields rc=4`
**Cause**: Advertisement payload too large.
**Fix**: Remove 128-bit service UUID from adv fields. Advertise only flags + TX power + name. Service is discoverable after connection via GATT.

### Build fails on `ble_store_config_init`
**Cause**: Symbol not available in this build configuration.
**Fix**: Remove `ble_store_config_init()` call. Keep `nimble_port_init`, GAP/GATT service init, and `ble_hs_cfg` callbacks.

### kconfig parsing errors
**Cause**: Old sdkconfig incompatible with newer ESP-IDF.
**Fix**: `idf.py -B build_clean set-target esp32c3 build` (regenerates config).

### GPIO register macro errors
**Error**: `GPIO_OUT_W1TS_REG` undefined.
**Cause**: Old register macros removed in ESP-IDF 6.x.
**Fix**: Use `gpio_ll_set_level(&GPIO, gpio_num, level)` instead.

### BLE not working after flash
**Cause**: Bluetooth not enabled in sdkconfig.
**Fix**: Ensure these are set in `sdkconfig`:
```
CONFIG_BT_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y
CONFIG_BT_CONTROLLER_ENABLED=y
```

### Serial monitor fails
**Error**: `No module named serial` or termios issues.
**Fix**: Use the project venv with pyserial installed (see Serial Log Capture above).

---

## Hardware Verification

If display doesn't render:
- Confirm app uses `FfxDisplaySpiBus2_nocs` as the SPI bus
- Confirm DC pin is `4` and reset pin is `5`
- Check that the correct app ID was flashed

If buttons don't respond:
- Check GPIO pins: 10, 8, 3, 2 (buttons 1-4)
- Buttons are active-low with pull-up resistors
- Use `edge_mask` for single-press, `pressed_mask` for held state
