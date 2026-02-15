#!/usr/bin/env python3
import argparse
import asyncio
import json
import math
import urllib.request

from bleak import BleakClient, BleakScanner

DEFAULT_NAME = "Pixie BLE Test"
DEFAULT_CHAR = "1130a240-d747-22b3-af44-4d8c106c7214"
USDC_ETH = "0xA0b86991c6218b36c1d19D4a2e9Eb0cE3606eB48"


def _format_cap(value: float) -> str:
    if value >= 1_000_000_000_000:
        return f"{value/1_000_000_000_000:.2f}T"
    if value >= 1_000_000_000:
        return f"{value/1_000_000_000:.2f}B"
    if value >= 1_000_000:
        return f"{value/1_000_000:.2f}M"
    if value >= 1_000:
        return f"{value/1_000:.2f}K"
    return f"{value:.0f}"


def _fetch_usdc_market_cap() -> tuple[str, float]:
    url = f"https://api.dexscreener.com/token-pairs/v1/ethereum/{USDC_ETH}"
    req = urllib.request.Request(url, headers={"User-Agent": "PixieBleClient/1.0"})
    with urllib.request.urlopen(req, timeout=10) as resp:
        data = json.loads(resp.read().decode("utf-8"))

    if not data:
        raise RuntimeError("No pairs returned for USDC")

    def liquidity_usd(item):
        try:
            return float(item.get("liquidity", {}).get("usd", 0))
        except Exception:
            return 0.0

    best = max(data, key=liquidity_usd)
    symbol = best.get("baseToken", {}).get("symbol", "USDC")

    mcap = best.get("marketCap")
    if mcap is None or (isinstance(mcap, float) and math.isnan(mcap)):
        mcap = best.get("fdv")
    if mcap is None:
        raise RuntimeError("No marketCap/fdv field in token pair")

    return symbol, float(mcap)


async def _find_device_address(name: str) -> str:
    devices = await BleakScanner.discover(timeout=6.0)
    for dev in devices:
        if dev.name == name:
            return dev.address
    raise RuntimeError(f"BLE device '{name}' not found")


async def _write_text(name: str, char_uuid: str, text: str) -> None:
    address = await _find_device_address(name)
    async with BleakClient(address) as client:
        await client.write_gatt_char(char_uuid, text.encode("utf-8"), response=False)


def main() -> int:
    parser = argparse.ArgumentParser(description="Fetch USDC market cap and send to Pixie over BLE.")
    parser.add_argument("--name", default=DEFAULT_NAME, help="BLE device name")
    parser.add_argument("--char", default=DEFAULT_CHAR, help="GATT characteristic UUID")
    args = parser.parse_args()

    symbol, mcap = _fetch_usdc_market_cap()
    text = f"{symbol} ${_format_cap(mcap)}"
    asyncio.run(_write_text(args.name, args.char, text))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
