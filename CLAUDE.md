# Firefly Pixie — Claude Code Project

This project provides Claude Code skills and firmware for the Firefly Pixie device (ESP32-C3).

## Project Layout

- `firmware/` — ESP-IDF firmware project (build and flash from here)
- `firmware/main/apps/` — App source files (one `.c` file per app)
- `firmware/pixie.sh` — Build and flash script
- `tools/pixie_ble_bridge/` — HTTP-to-BLE bridge for sending messages
- `skills/` — Claude Code skill definitions (install with `./install.sh`)

## Key Commands

```bash
cd firmware
./pixie.sh list                        # List available apps
./pixie.sh build <app>                 # Build an app
./pixie.sh flash <app>                 # Build + flash
./pixie.sh flash <app> /dev/cu.usbmodem101  # Flash with explicit port
```

## Critical Rules

1. **Always build via Docker** — the build uses `espressif/idf` container image
2. **Always flash from host** — Docker serial passthrough doesn't work on macOS
3. **Fragment rendering** — display is 240x240 split into 10 strips of 240x24
4. **App contract** — every app needs: `init`, `tick`, `render_fragment`, `deinit`
5. **Register new apps** in both `firmware/main/CMakeLists.txt` AND `firmware/pixie.sh`

## Known Pitfalls

- Old sdkconfig files break kconfig parsing — use `set-target esp32c3` to regenerate
- BLE adv payload must be minimal (no 128-bit UUID in adv fields) or you get `rc=4`
- ESP-IDF 6.x needs `esp_driver_gpio` and `esp_driver_spi` in component deps
- Use `gpio_ll_set_level` instead of old register macros for GPIO
