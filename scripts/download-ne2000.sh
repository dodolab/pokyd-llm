#!/usr/bin/env bash
# Download NE2000.COM (Crynwr packet driver collection) into assets/.
# Primary mirror: Internet Archive snapshot of crynwr.com (original site may be offline).
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT_DIR/assets/NE2000.COM"
TMPZIP="$(mktemp)"

cleanup() { rm -f "$TMPZIP"; }
trap cleanup EXIT

ARCHIVE_URL="https://web.archive.org/web/20060209004759/http://www.crynwr.com/drivers/pktd11.zip"

echo "Downloading packet driver archive..."
curl -fsSL -o "$TMPZIP" "$ARCHIVE_URL"

unzip -p "$TMPZIP" NE2000.COM > "$OUT"
chmod -x "$OUT" 2>/dev/null || true
echo "Wrote $OUT ($(wc -c < "$OUT") bytes)."
echo "Usage in DOSBox-X (same line as build-and-run.sh):"
echo "  NE2000.COM 0x60 10 0x300"
