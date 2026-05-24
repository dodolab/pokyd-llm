#!/usr/bin/env bash
# Download Open Watcom v2 snapshot into ./watcom (macOS / Linux). Same archive as CI.
#
# Usage:
#   ./scripts/install-open-watcom.sh
#   ./scripts/install-open-watcom.sh /opt/watcom
#
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="${1:-$ROOT_DIR/watcom}"
URL="https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/ow-snapshot.tar.xz"
TMP="$(mktemp)"

cleanup() { rm -f "$TMP"; }
trap cleanup EXIT

mkdir -p "$DEST"
echo "Downloading Open Watcom snapshot..."
if command -v wget >/dev/null 2>&1; then
  wget -q -O "$TMP" "$URL"
elif command -v curl >/dev/null 2>&1; then
  curl -fsSL -o "$TMP" "$URL"
else
  echo "ERROR: need wget or curl to download the snapshot"
  exit 1
fi
echo "Extracting into $DEST ..."
tar xf "$TMP" -C "$DEST"

if [[ -x "$DEST/binl64/wcl" ]]; then
  echo "OK: $DEST/binl64/wcl"
elif [[ -x "$DEST/binl/wcl" ]]; then
  echo "OK: $DEST/binl/wcl"
elif [[ -x "$DEST/armo64/wcl" ]]; then
  echo "OK: $DEST/armo64/wcl"
elif [[ -x "$DEST/arml64/wcl" ]]; then
  echo "OK: $DEST/arml64/wcl"
else
  echo "ERROR: no wcl found under $DEST"
  exit 1
fi

echo "Set: export WATCOM=$DEST"
