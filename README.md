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

## Cool Things to Try

Open Claude Code in this project and paste any of these prompts. Claude handles the rest — creates the app, registers it, builds, and flashes.

### Games

> **"Create a snake game. Button 1 = left, Button 2 = up, Button 3 = down, Button 4 = right. Draw the snake with green rectangles and food with a red square. Show the score at the top."**

> **"Build a Pong game. Buttons 1 and 2 control the left paddle, buttons 3 and 4 control the right paddle. Ball bounces off walls and paddles. Show the score centered at the top."**

> **"Make a tic-tac-toe game for two players. Use buttons 1-3 to pick a column and button 4 to confirm. Highlight the current position. Show X and O on the grid."**

> **"Create a brick breaker game. The paddle moves left and right with buttons 1 and 4. Ball bounces off bricks drawn as small colored rectangles across the top."**

### Party & Social

> **"Build a truth or dare app. Show 'Truth' or 'Dare' on screen. Press any button to randomly pick a new prompt from a built-in list. Alternate between truth and dare questions. Make the background pulse while waiting."**

> **"Create a magic 8-ball app. Display 'Ask a question...' then when any button is pressed, show a random answer like 'Yes definitely', 'Ask again later', 'Don't count on it' with a cool animation."**

> **"Make a dice roller. Show a big dice face on screen using dots drawn with rectangles. Press any button to roll — animate the number cycling for a second before landing on the result."**

> **"Build a 'Never Have I Ever' app. Show prompts one at a time. Button 1 = next prompt, Button 4 = shuffle all. Keep a running counter of how many rounds have been played."**

### Utility

> **"Create a stopwatch app. Button 1 starts and stops the timer. Button 4 resets it. Display minutes, seconds, and milliseconds in large centered text. Show lap times when Button 2 is pressed."**

> **"Build a reaction time tester. Show 'Wait...' with a red background, then after a random 1-5 second delay, flash green and say 'GO!'. Measure how fast the user presses any button. Show their time in milliseconds."**

> **"Make a countdown timer. Start at 60 seconds. Button 1 adds 10 seconds, Button 4 subtracts 10 seconds, Button 2 starts/pauses, Button 3 resets. Flash the screen red when time is up."**

> **"Create a Pomodoro timer. 25 minutes of focus, 5 minutes of break. Show the remaining time big and centered. Pulse green during focus mode, blue during break. Button 1 to start/pause, Button 4 to skip."**

### Visual & Creative

> **"Build a digital pet app like a Tamagotchi. Show a simple face that changes expression. It gets hungry over time (show a hunger bar). Button 1 feeds it, Button 2 plays with it, Button 3 puts it to sleep. If neglected, it gets sad."**

> **"Create a pixel art canvas. Use a 16x16 grid on the 240x240 screen. Buttons 1 and 2 move the cursor left/right, Button 3 moves down, Button 4 toggles the pixel. Cycle through colors on double-press."**

> **"Make a rainbow wave animation that flows across the screen. No buttons needed — just a mesmerizing color cycling effect. Use the full display with smooth gradients."**

### BLE-Powered

> **"Set up the BLE demo app and create a script that sends the current time to the Pixie every 10 seconds using the BLE bridge."**

> **"Flash the BLE demo and send a message that says 'gm' with a rainbow mode background."**

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
