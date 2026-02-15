# Build & Flash Troubleshooting

## sdkconfig Breaks Build

**Symptom**: kconfig parsing errors during build.

**Cause**: Old `sdkconfig` files are incompatible with newer ESP-IDF versions.

**Fix**: Let ESP-IDF regenerate the config:
```bash
# Inside Docker or via pixie.sh
idf.py -B build_clean set-target esp32c3 build
```
The `set-target` step regenerates a fresh sdkconfig.

---

## Docker Serial Passthrough Fails

**Symptom**: `--device /dev/cu.usbmodem101` inside Docker doesn't work on macOS.

**Cause**: macOS doesn't support USB serial passthrough to Docker containers reliably.

**Fix**: Always flash from the host, not from inside Docker. `pixie.sh` already handles this correctly -- it builds in Docker and flashes from host.

---

## esptool Not Found

**Symptom**: `esptool: command not found` or `No module named esptool`.

**Fix**: `pixie.sh` auto-creates a venv. If it fails, do it manually:
```bash
cd firmware
python3 -m venv .venv-esptool
.venv-esptool/bin/pip install esptool
```

---

## No Serial Port Detected

**Symptom**: `No serial port detected` error from `pixie.sh`.

**Fix**:
1. Check the USB cable is connected
2. Check the device is powered on
3. List available ports: `ls /dev/cu.usb*`
4. Pass port explicitly: `./pixie.sh flash handshake /dev/cu.usbmodem101`

---

## Flash Succeeds But Device Unchanged

**Possible causes**:
1. Wrong app was flashed (check the app ID)
2. Device needs power cycle -- unplug and replug USB
3. Build artifacts are stale -- delete `build_clean/` and rebuild

---

## ESP-IDF 6.x Compatibility

If building components from scratch, these are required for ESP-IDF 6.x:
- Add `esp_driver_gpio` and `esp_driver_spi` to component dependencies in `CMakeLists.txt`
- Replace old register macros (`GPIO_OUT_W1TS_REG`) with `gpio_ll_set_level(&GPIO, pin, level)`
- These fixes are already applied in the vendored components
