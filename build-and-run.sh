#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOSBOX_X_BIN="${NOTES_DOSBOX_X:-}"
EXIT_AFTER_POKYD=0

for arg in "$@"; do
  case "$arg" in
    --exit-after-pokyd)
      EXIT_AFTER_POKYD=1
      ;;
    *)
      echo "Unknown argument: $arg"
      echo "Usage: ./build-and-run.sh [--exit-after-pokyd]"
      exit 1
      ;;
  esac
done

if [[ -z "$DOSBOX_X_BIN" ]]; then
  if command -v dosbox-x >/dev/null 2>&1; then
    DOSBOX_X_BIN="$(command -v dosbox-x)"
  fi
fi

if [[ -z "$DOSBOX_X_BIN" || ! -x "$DOSBOX_X_BIN" ]]; then
  echo "DOSBox-X not found. Install dosbox-x or set NOTES_DOSBOX_X to its full path."
  exit 1
fi

echo "Building with Open Watcom (macOS host + DOSBox-X guest)..."
"$ROOT_DIR/build.sh"

CONF_PATH="$(mktemp -t pokyd-run)"
trap 'rm -f "$CONF_PATH"' EXIT

AUTOEXEC_TAIL=$(cat <<'EOF'
if not exist C:\SLOVNIK.DAT if exist C:\assets\slova.pkd copy C:\assets\slova.pkd C:\SLOVA.PKD >nul
if not exist C:\SLOVNIK.DAT if exist C:\assets\pokydx.pkd copy C:\assets\pokydx.pkd C:\POKYDX.PKD >nul
if not exist C:\SLOVNIK.DAT if exist C:\slova.exe slova.exe
echo [pokyd] Starting pokyd.exe ...
pokyd.exe
echo.
echo pokyd.exe ended with errorlevel %ERRORLEVEL%
echo Staying at C:\ prompt --- type EXIT to close DOSBox.
EOF
)

if [[ "$EXIT_AFTER_POKYD" -eq 1 ]]; then
  AUTOEXEC_TAIL=$(cat <<'EOF'
pokyd.exe
exit
EOF
)
fi

cat >"$CONF_PATH" <<EOF
[sdl]
output=surface
usescancodes=false
[render]
aspect=true
doublescan=false
[dosbox]
title=Pokyd DOS (macOS)
keyboard hook=false
[cpu]
core=normal
[dos]
ver=7.1
lfn=true
keyboardlayout=us
[autoexec]
@echo off
echo [pokyd] Autoexec started
mount c "$ROOT_DIR"
c:
echo [pokyd] Switched to drive C
SET PATH=C:\
$AUTOEXEC_TAIL
EOF

echo
echo "Starting DOSBox-X..."
"$DOSBOX_X_BIN" -conf "$CONF_PATH"
