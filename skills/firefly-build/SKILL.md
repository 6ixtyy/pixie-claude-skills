---
name: firefly-build
description: Build and flash Firefly Pixie firmware. Use when building apps, flashing firmware, setting up Docker, detecting serial ports, running pixie.sh, or deploying to the ESP32-C3 device.
---

# Firefly Build & Flash

Build and flash firmware apps to the Firefly Pixie (ESP32-C3) device.

---

## Prerequisites

- **Docker**: Required for the ESP-IDF 6.1 build environment (`espressif/idf` image)
- **Python 3**: Required for `esptool` (host-side flashing)
- **USB cable**: Connect Pixie via USB-C

## Quick Commands

All commands run from the `firmware/` directory:

```bash
cd firmware

# List available apps
./pixie.sh list

# Build an app (Docker-based)
./pixie.sh build <app_id>

# Build + flash an app
./pixie.sh flash <app_id>

# Flash with explicit serial port
./pixie.sh flash <app_id> /dev/cu.usbmodem101
```

## Available Apps

| App ID | Description |
|--------|-------------|
| `handshake` | Button pattern unlock demo |
| `template` | Minimal starter app (copy this for new apps) |
| `valentine` | Animated hearts with theme cycling |
| `ble_demo` | BLE text/color/button interaction via phone |

## How the Build Works

1. **Docker build**: `pixie.sh` mounts `firmware/` into a Docker container running `espressif/idf`
2. **CMake selects app**: `-DPIXIE_APP=<id>` picks which `app_*.c` file to compile
3. **Output**: Build artifacts go to `firmware/build_clean/`
4. **Flash from host**: `esptool` runs on macOS (not inside Docker) because serial passthrough fails on macOS

### Build internals

```
Docker: espressif/idf
  ↓ mounts firmware/ as /work
  ↓ runs: idf.py -B build_clean -DPIXIE_APP=<id> set-target esp32c3 build
  ↓ output: build_clean/

Host: esptool
  ↓ reads: build_clean/flash_args
  ↓ flashes via: /dev/cu.usbmodem*
```

## Serial Port Detection

The Pixie shows up as an Espressif USB JTAG/Serial device:

```bash
# Auto-detect
ls /dev/cu.usbmodem*

# Common ports on macOS
/dev/cu.usbmodem101
/dev/cu.usbmodem1101
```

If the port changes after re-plugging, pass it explicitly:
```bash
./pixie.sh flash handshake /dev/cu.usbmodem1101
```

## esptool Setup

`pixie.sh` auto-creates a Python venv with `esptool` if needed. To set it up manually:

```bash
cd firmware
python3 -m venv .venv-esptool
source .venv-esptool/bin/activate
pip install esptool
```

## Adding a New App to the Build

After creating a new `app_<id>.c` file:

1. **Register in CMakeLists.txt** (`firmware/main/CMakeLists.txt`):
   ```cmake
   elseif(PIXIE_APP STREQUAL "myapp")
     set(PIXIE_APP_SOURCE "apps/app_myapp.c")
   ```

2. **Register in pixie.sh** (`firmware/pixie.sh`):
   ```bash
   APPS=("handshake" "template" "valentine" "ble_demo" "myapp")
   ```

3. **Build and flash**:
   ```bash
   ./pixie.sh flash myapp
   ```

## Verification

A successful flash shows:
```
Connected to ESP32-C3
Hash of data verified.
Hard resetting via RTS pin...
```
