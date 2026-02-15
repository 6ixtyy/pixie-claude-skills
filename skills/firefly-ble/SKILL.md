---
name: firefly-ble
description: Interact with Firefly Pixie over Bluetooth Low Energy. Use when sending BLE messages, controlling colors, setting up the BLE bridge, using pixie_ble_bridge, or connecting to the Pixie from a phone or computer.
---

# Firefly BLE Interaction

Send messages, change colors, and receive button presses from the Pixie over BLE.

---

## BLE Architecture

The `ble_demo` app runs a NimBLE GATT server on the Pixie with:

| Characteristic | UUID | Type | Description |
|---------------|------|------|-------------|
| Message | `1130a240-d747-22b3-af44-4d8c106c7214` | Read/Write | UTF-8 text (max 32 chars) |
| Color | `1130a240-d747-22b3-af44-4d8c106c7215` | Read/Write | 3-byte RGB (e.g., `FF4A90`) |
| Button | `1130a240-d747-22b3-af44-4d8c106c7216` | Read/Notify | Reports A/B/C/D presses |

**Service UUID**: `1130a240-d747-22b3-af44-4d8c106c7213`
**Device name**: `Pixie BLE Test`

---

## Option 1: Phone App (nRF Connect / LightBlue)

1. Flash the BLE demo: `cd firmware && ./pixie.sh flash ble_demo`
2. Open nRF Connect or LightBlue on your phone
3. Scan and connect to `Pixie BLE Test`
4. Find service `1130a240-...7213`
5. Write text to characteristic `...7214`
6. Write RGB hex to characteristic `...7215`
7. Enable notifications on `...7216` for button events

---

## Option 2: HTTP-to-BLE Bridge (Computer)

The bridge runs a local HTTP server that forwards requests to the Pixie over BLE.

### Setup

```bash
cd tools/pixie_ble_bridge
./pixie_ble_bridge.sh
```

This auto-creates a Python venv with `bleak` and starts an HTTP server on port 8765.

### Send Text

```bash
curl -X POST http://localhost:8765 \
  -H "Content-Type: application/json" \
  -d '{"text": "Hello Pixie"}'
```

### Send Commands

```bash
# Visual modes
curl -X POST http://localhost:8765 -d '{"text": "/mode static"}'
curl -X POST http://localhost:8765 -d '{"text": "/mode pulse"}'
curl -X POST http://localhost:8765 -d '{"text": "/mode rainbow"}'

# DEX display
curl -X POST http://localhost:8765 -d '{"text": "/dex top BTC 2T"}'

# Help
curl -X POST http://localhost:8765 -d '{"text": "/help"}'
```

### Send Crypto Market Data

```bash
cd tools/pixie_ble_bridge
./send_usdc_mcap.sh
```

This fetches USDC market cap from DexScreener and displays it on the Pixie.

---

## BLE Commands Reference

Write these to the message characteristic (`...7214`):

| Command | Effect |
|---------|--------|
| `/mode static` | Solid background color |
| `/mode pulse` | Pulsing brightness animation |
| `/mode rainbow` | Cycling rainbow background |
| `/help` | Display available modes |
| `/dex top` | Show command format help |
| `/dex top BTC 2T` | Display "BTC $2T" on screen |
| Any other text | Display as-is on screen |

---

## BLE Bridge Script Details

`pixie_ble_bridge.py` options:

```bash
python3 pixie_ble_bridge.py \
  --host 0.0.0.0 \      # Bind address (default: 0.0.0.0)
  --port 8765 \          # HTTP port (default: 8765)
  --name "Pixie BLE Test" \  # BLE device name to scan for
  --char "1130a240-d747-22b3-af44-4d8c106c7214"  # Write characteristic UUID
```

---

## Display Behavior

- Short messages (1-10 chars): Large centered text (scale 4)
- Medium messages (11-14 chars): Medium centered text (scale 3)
- Longer messages (15-32 chars): Splits across two lines (scale 2)
- Top-right corner: Green dot = connected, red dot = disconnected
