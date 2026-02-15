#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${SCRIPT_DIR}"
BUILD_DIR="${PROJECT_DIR}/build_clean"
ESPVENV_DIR="${PROJECT_DIR}/.venv-esptool"
APPS=("handshake" "template" "valentine" "ble_demo")

usage() {
  cat <<EOF
Usage:
  ./pixie.sh list
  ./pixie.sh build <app>
  ./pixie.sh flash <app> [port]

Examples:
  ./pixie.sh list
  ./pixie.sh build handshake
  ./pixie.sh flash template /dev/cu.usbmodem101
  ./pixie.sh flash valentine
  ./pixie.sh flash ble_demo
EOF
}

is_valid_app() {
  local value="$1"
  for candidate in "${APPS[@]}"; do
    if [[ "${candidate}" == "${value}" ]]; then
      return 0
    fi
  done
  return 1
}

pick_port() {
  local selected_port="${1:-}"
  if [[ -n "${selected_port}" ]]; then
    echo "${selected_port}"
    return
  fi

  local detected
  detected="$(ls /dev/cu.usbmodem* 2>/dev/null | head -n 1 || true)"
  if [[ -z "${detected}" ]]; then
    echo "No serial port detected. Pass one explicitly as the third arg." >&2
    exit 1
  fi
  echo "${detected}"
}

run_build() {
  local app="$1"
  docker run --rm \
    -v "${PROJECT_DIR}:/work" \
    -w "/work" \
    -e HOME=/tmp \
    espressif/idf \
    idf.py -B build_clean -DPIXIE_APP="${app}" set-target esp32c3 build
}

ensure_esptool() {
  if command -v esptool >/dev/null 2>&1 || command -v esptool.py >/dev/null 2>&1; then
    echo "python3"
    return
  fi

  if [[ ! -d "${ESPVENV_DIR}" ]]; then
    python3 -m venv "${ESPVENV_DIR}"
  fi
  "${ESPVENV_DIR}/bin/pip" install -q esptool
  echo "${ESPVENV_DIR}/bin/python"
}

run_flash() {
  local app="$1"
  local port="$2"

  run_build "${app}"

  local pybin
  pybin="$(ensure_esptool)"

  (
    cd "${BUILD_DIR}"
    if [[ "${pybin}" == "python3" ]]; then
      python3 -m esptool --chip esp32c3 -p "${port}" -b 460800 \
        --before default-reset --after hard-reset write_flash @flash_args
    else
      "${pybin}" -m esptool --chip esp32c3 -p "${port}" -b 460800 \
        --before default-reset --after hard-reset write_flash @flash_args
    fi
  )
}

if [[ $# -lt 1 ]]; then
  usage
  exit 1
fi

command="$1"
case "${command}" in
  list)
    printf '%s\n' "${APPS[@]}"
    ;;
  build)
    if [[ $# -ne 2 ]]; then
      usage
      exit 1
    fi
    app="$2"
    if ! is_valid_app "${app}"; then
      echo "Unknown app '${app}'. Supported: ${APPS[*]}" >&2
      exit 1
    fi
    run_build "${app}"
    ;;
  flash)
    if [[ $# -lt 2 || $# -gt 3 ]]; then
      usage
      exit 1
    fi
    app="$2"
    if ! is_valid_app "${app}"; then
      echo "Unknown app '${app}'. Supported: ${APPS[*]}" >&2
      exit 1
    fi
    port="$(pick_port "${3:-}")"
    echo "Using serial port: ${port}"
    run_flash "${app}" "${port}"
    ;;
  *)
    usage
    exit 1
    ;;
esac
