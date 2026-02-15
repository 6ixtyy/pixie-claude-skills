---
name: firefly-hardware
description: Firefly Pixie hardware specifications. Use when asking about pins, GPIOs, display specs, memory, ESP32-C3 capabilities, button wiring, LED configuration, or device specifications.
---

# Firefly Pixie Hardware Reference

---

## Device Overview

| Spec | Value |
|------|-------|
| **MCU** | ESP32-C3 (RISC-V, single-core, 160MHz) |
| **RAM** | 400KB SRAM |
| **Flash** | 16MB |
| **Display** | ST7789 1.3" IPS, 240x240, 16-bit RGB565 |
| **Buttons** | 4x tactile switches |
| **LEDs** | 4x WS2812B RGB |
| **Connectivity** | BLE 5.0, USB-C (JTAG/Serial) |
| **Framework** | ESP-IDF 6.1 |

---

## Pin Mapping

### Display (SPI)
| Function | GPIO |
|----------|------|
| Data/Command (DC) | 4 |
| Reset | 5 |
| SPI Bus | `FfxDisplaySpiBus2_nocs` (pre-configured) |

### Buttons (Active Low, Internal Pull-Up)
| Button | Index | GPIO | Position |
|--------|-------|------|----------|
| Button 1 | 0 | 10 | Leftmost |
| Button 2 | 1 | 8 | Center-left |
| Button 3 | 2 | 3 | Center-right |
| Button 4 | 3 | 2 | Rightmost |

Buttons read LOW when pressed (active-low with internal pull-up resistors).

---

## Display Details

- **Resolution**: 240 x 240 pixels
- **Color format**: RGB565 (16-bit, 65K colors)
- **Rendering**: Fragment-based (240x24 strips, 10 per frame)
- **Fragment height**: 24 pixels (`FfxDisplayFragmentHeight`)
- **Frame rate**: ~60fps target (16ms per frame)
- **Orientation**: `FfxDisplayRotationRibbonRight`

### Memory per fragment
- `240 * 24 * 2 bytes = 11,520 bytes` (~11.25 KB)
- Only one fragment in memory at a time (saves RAM)

---

## BLE

- **Stack**: NimBLE (Apache Mynewt)
- **Roles**: Peripheral (advertiser), GATT server
- **Max connections**: 1 (single phone at a time)
- **Host stack size**: 4096 bytes
- **Max bonds**: 3
- **Advertisement**: Flags + TX power + device name (keep under 31 bytes total)

---

## USB

- **Interface**: USB-C
- **Protocol**: USB JTAG/Serial (Espressif built-in)
- **macOS device path**: `/dev/cu.usbmodem*`
- **Baud rate**: 115200 (serial monitor), 460800 (flashing)

---

## Memory Constraints

- **Total SRAM**: 400KB (shared between app, BLE stack, FreeRTOS, display buffer)
- **Display fragment buffer**: ~11.5KB
- **BLE NimBLE stack**: ~30-50KB
- **Available for app logic**: ~250-300KB
- **Flash**: 16MB (plenty for code + assets)

### Partition Layout
The flash is divided into partitions for bootloader, app code, NVS storage, and OTA.

---

## Power

- **USB powered**: Device draws power from USB-C connection
- **No battery**: Always tethered via USB cable

---

## Build Environment

- **Toolchain**: ESP-IDF 6.1 (via Docker `espressif/idf`)
- **Target**: `esp32c3`
- **Architecture**: RISC-V
- **Build system**: CMake + Ninja (inside Docker)
- **Flash tool**: esptool (runs on host macOS)
