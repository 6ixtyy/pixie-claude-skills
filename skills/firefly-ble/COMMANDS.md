# BLE Commands Reference

## GATT Service

- **Service UUID**: `1130a240-d747-22b3-af44-4d8c106c7213`
- **Device Name**: `Pixie BLE Test`

## Characteristics

### Message (Read/Write)
- **UUID**: `1130a240-d747-22b3-af44-4d8c106c7214`
- **Max length**: 32 bytes UTF-8
- **Write**: Sets display text or executes a command
- **Read**: Returns current display text

### Color (Read/Write)
- **UUID**: `1130a240-d747-22b3-af44-4d8c106c7215`
- **Format**: 3 bytes: `[R, G, B]` (0-255 each)
- **Write**: Sets background color
- **Read**: Returns current background RGB

### Button (Read/Notify)
- **UUID**: `1130a240-d747-22b3-af44-4d8c106c7216`
- **Notify payload**: `"A"`, `"B"`, `"C"`, or `"D"`
- **Read**: Returns last pressed button name

## Message Commands

| Command | Description |
|---------|-------------|
| `/mode static` | Solid color background |
| `/mode pulse` | Pulsing brightness on current color |
| `/mode rainbow` | Cycling rainbow colors |
| `/help` | Show available modes on screen |
| `/dex top` | Show DEX command format |
| `/dex top <SYM> <MCAP>` | Display ticker (e.g., `/dex top ETH 450B`) |

## Color Examples

| Hex Bytes | Color |
|-----------|-------|
| `FF0000` | Red |
| `00FF00` | Green |
| `0000FF` | Blue |
| `FF4A90` | Pink |
| `200012` | Dark purple (default) |
