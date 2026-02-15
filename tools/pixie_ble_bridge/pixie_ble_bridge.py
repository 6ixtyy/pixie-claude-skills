#!/usr/bin/env python3
import asyncio
import json
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from typing import Optional

from bleak import BleakClient, BleakScanner

DEFAULT_NAME = "Pixie BLE Test"
DEFAULT_CHAR = "1130a240-d747-22b3-af44-4d8c106c7214"

_loop: Optional[asyncio.AbstractEventLoop] = None
_client: Optional[BleakClient] = None
_device_name = DEFAULT_NAME
_char_uuid = DEFAULT_CHAR
_lock = asyncio.Lock()


async def _find_device_address(name: str) -> Optional[str]:
    devices = await BleakScanner.discover(timeout=5.0)
    for dev in devices:
        if dev.name == name:
            return dev.address
    return None


async def _get_client() -> BleakClient:
    global _client
    async with _lock:
        if _client and _client.is_connected:
            return _client

        address = await _find_device_address(_device_name)
        if not address:
            raise RuntimeError(f"BLE device '{_device_name}' not found")

        _client = BleakClient(address)
        await _client.connect()
        return _client


async def write_text(text: str) -> None:
    if not text:
        return
    client = await _get_client()
    await client.write_gatt_char(_char_uuid, text.encode("utf-8"), response=False)


class _Handler(BaseHTTPRequestHandler):
    def _send(self, code: int, body: str) -> None:
        self.send_response(code)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body.encode("utf-8"))))
        self.end_headers()
        self.wfile.write(body.encode("utf-8"))

    def do_POST(self):
        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length) if length > 0 else b""
        text = ""

        if raw:
            try:
                payload = json.loads(raw.decode("utf-8"))
                text = payload.get("text", "")
            except Exception:
                text = raw.decode("utf-8", errors="replace")

        if not text:
            self._send(400, "Missing text")
            return

        fut = asyncio.run_coroutine_threadsafe(write_text(text), _loop)
        try:
            fut.result(timeout=10)
            self._send(200, "OK")
        except Exception as exc:
            self._send(500, f"ERR: {exc}")

    def log_message(self, format, *args):
        return


def _run_http(host: str, port: int):
    httpd = HTTPServer((host, port), _Handler)
    httpd.serve_forever()


def main():
    import argparse

    parser = argparse.ArgumentParser(description="Pixie BLE bridge (HTTP -> BLE).")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--name", default=DEFAULT_NAME)
    parser.add_argument("--char", default=DEFAULT_CHAR)
    args = parser.parse_args()

    global _loop, _device_name, _char_uuid
    _device_name = args.name
    _char_uuid = args.char

    _loop = asyncio.new_event_loop()
    asyncio.set_event_loop(_loop)

    thread = threading.Thread(target=_run_http, args=(args.host, args.port), daemon=True)
    thread.start()

    _loop.run_forever()


if __name__ == "__main__":
    main()
