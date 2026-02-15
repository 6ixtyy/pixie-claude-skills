# Pixie Claude Skills

Claude Code skills for building apps on the [Firefly](https://github.com/firefly) Pixie device (ESP32-C3).

Clone this repo, run the installer, and start building firmware apps with Claude Code — no manual setup required.

## What's Inside

- **5 Claude Code skills** — build, create apps, BLE interaction, debugging, hardware reference
- **Complete firmware** — multi-app ESP-IDF project, ready to build and flash
- **BLE bridge tools** — send messages to the Pixie from your computer over Bluetooth
- **One-command install** — skills are copied to your Claude Code config automatically

## Prerequisites

- [Docker](https://www.docker.com/) (for ESP-IDF build environment)
- Python 3
- USB-C cable
- A Firefly Pixie device

## Quick Start

```bash
# 1. Clone the repo
git clone https://github.com/6ixtyy/pixie-claude-skills.git
cd pixie-claude-skills

# 2. Install the Claude Code skills
./install.sh

# 3. Open the project in Claude Code and start building
claude

# Try these prompts:
#   "build and flash the template app"
#   "create a new app called countdown"
#   "send a BLE message to the Pixie"
#   "what pins are the buttons on?"
```

## Installed Skills

| Skill | Triggers On | What It Does |
|-------|-------------|--------------|
| `firefly-build` | "build", "flash", "deploy" | Build and flash firmware apps |
| `firefly-app-dev` | "create app", "new app", "write firmware" | Step-by-step app creation with API reference |
| `firefly-ble` | "BLE", "bluetooth", "send message" | BLE interaction via phone or HTTP bridge |
| `firefly-debug` | "error", "debug", "not working" | Diagnose build, flash, and runtime issues |
| `firefly-hardware` | "pins", "GPIO", "specs", "hardware" | Hardware specs and pin mapping |

## Project Layout

```
pixie-claude-skills/
├── firmware/              # ESP-IDF project (build from here)
│   ├── pixie.sh           # Build & flash script
│   ├── main/apps/         # App source files
│   └── components/        # Firefly display, crypto, storage libs
├── tools/                 # BLE bridge utilities
│   └── pixie_ble_bridge/
├── skills/                # Claude Code skill definitions
└── install.sh             # One-command installer
```

## Available Apps

| App | Description |
|-----|-------------|
| `handshake` | Button pattern unlock demo |
| `template` | Minimal starter — copy this for new apps |
| `valentine` | Animated hearts with theme cycling |
| `ble_demo` | BLE text/color/button control via phone |

## Manual Commands

```bash
cd firmware

# List apps
./pixie.sh list

# Build
./pixie.sh build handshake

# Flash (auto-detects serial port)
./pixie.sh flash template

# Flash with explicit port
./pixie.sh flash ble_demo /dev/cu.usbmodem101
```

## BLE Quick Start

```bash
# Flash the BLE app
cd firmware && ./pixie.sh flash ble_demo

# Start the HTTP-to-BLE bridge
cd ../tools/pixie_ble_bridge && ./pixie_ble_bridge.sh

# Send a message
curl -X POST http://localhost:8765 -d '{"text": "Hello Pixie"}'
```

## Credits

Built for the [Firefly Pixie](https://github.com/firefly) hardware by the Firefly team ([@FireflyPocket](https://x.com/FireflyPocket)).

The firmware components (`firefly-display`, `firefly-ethers`, `firefly-hollows`, `firefly-scene`) are from the Firefly project and are licensed under MIT.

## License

MIT
