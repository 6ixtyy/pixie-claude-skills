#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PYTHON_BIN="/usr/bin/python3"
if [[ ! -x "${PYTHON_BIN}" ]]; then
  PYTHON_BIN="python3"
fi
VENV_DIR="${ROOT_DIR}/.venv-ble-bridge-py39"

if [[ ! -d "${VENV_DIR}" ]]; then
  "${PYTHON_BIN}" -m venv "${VENV_DIR}"
fi

"${VENV_DIR}/bin/pip" install -q bleak

exec "${VENV_DIR}/bin/python" \
  "${ROOT_DIR}/tools/pixie_ble_bridge/pixie_ble_bridge.py" "$@"
