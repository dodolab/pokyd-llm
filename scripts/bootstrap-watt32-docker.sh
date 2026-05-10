#!/usr/bin/env bash
# Rebuild vendor/watt32-dos from Watt-32 sources using Docker (linux/amd64).
# Prerequisites: Docker, network access (GitHub + Ubuntu mirrors).
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VENDOR_SRC="$ROOT_DIR/vendor/watt-32"
OUT_DIR="$ROOT_DIR/vendor/watt32-dos"
INNER_SH="$SCRIPT_DIR/watt32-docker-build-inner.sh"
if [[ ! -f "$INNER_SH" ]]; then
  echo "Missing inner script: $INNER_SH"
  exit 1
fi

# Windows checkouts often use CRLF; bash in the container then sees $'\r' and breaks set/apt-get.
if [[ -n "${WINDIR:-}" ]] && command -v powershell.exe >/dev/null 2>&1; then
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$SCRIPT_DIR/ensure-sh-lf.ps1" -Path "$INNER_SH" || true
fi

echo "==> Checking Docker daemon..."
if ! docker info >/dev/null 2>&1; then
  echo "Cannot connect to the Docker daemon. Start Docker Desktop (or dockerd) and retry."
  echo "Tip: run 'docker version' and confirm a Server section is shown."
  exit 1
fi

echo "==> Cloning Watt-32 into vendor/watt-32 (shallow, reproducible)..."
mkdir -p "$ROOT_DIR/vendor"
rm -rf "$VENDOR_SRC"
git clone --depth 1 https://github.com/gvanem/Watt-32.git "$VENDOR_SRC"

echo "==> Building wattcpwl.lib inside Docker (Ubuntu + Open Watcom snapshot + DOSBox)..."
docker run --rm --platform linux/amd64 \
  -e "SDL_VIDEODRIVER=dummy" \
  -v "$VENDOR_SRC:/watt" \
  -v "$INNER_SH:/inner.sh:ro" \
  ubuntu:22.04 bash /inner.sh

echo "==> Installing headers + libs into vendor/watt32-dos ..."
mkdir -p "$OUT_DIR/lib"
rm -rf "$OUT_DIR/inc"
cp -a "$VENDOR_SRC/inc" "$OUT_DIR/"
cp -f "$VENDOR_SRC/lib/wattcpwl.lib" "$OUT_DIR/lib/wattcpwl.lib"
cp -f "$OUT_DIR/lib/wattcpwl.lib" "$OUT_DIR/lib/wattcplf.lib"

echo "Done. Run ./build.sh from the repo root (WATT_ROOT defaults to vendor/watt32-dos)."
