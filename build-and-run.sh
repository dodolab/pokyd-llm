#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOSBOX_X_BIN="${NOTES_DOSBOX_X:-}"
EXIT_AFTER_POKYD=0
SKIP_BUILD=0
SKIP_INTRO=0
# Optional: set POKYD_LLM_HOST=host:port to enable LLM mode inside DOSBox-X.
# The Node bridge must be running on the HOST before launching DOSBox-X.
# Example: POKYD_LLM_HOST=10.0.2.2:8765 ./build-and-run.sh
POKYD_LLM_HOST="${POKYD_LLM_HOST:-}"

for arg in "$@"; do
  case "$arg" in
    --exit-after-pokyd)
      EXIT_AFTER_POKYD=1
      ;;
    --no-build)
      SKIP_BUILD=1
      ;;
    --skip-intro)
      SKIP_INTRO=1
      ;;
    --llm=*)
      POKYD_LLM_HOST="${arg#--llm=}"
      ;;
    *)
      echo "Unknown argument: $arg"
      echo "Usage: ./build-and-run.sh [--exit-after-pokyd] [--no-build] [--skip-intro] [--llm=host:port]"
      exit 1
      ;;
  esac
done

if [[ "$EXIT_AFTER_POKYD" -eq 1 ]]; then
  SKIP_INTRO=1
  echo "Note: --exit-after-pokyd is set: DOSBox-X will quit as soon as pokyd.exe exits"
  echo "      (non-interactive). Omit this flag to stay at the C:\\ prompt."
fi

# LLM + Open Watcom/Watt-32 use x87 FPU opcodes. core=normal can log
# "ERROR FPU:8087 only fpu code...". Default to core=dynamic when -llm is used;
# override with: POKYD_DOSBOX_CPU_CORE=normal ./build-and-run.sh ...
if [[ -n "$POKYD_LLM_HOST" ]]; then
  DOSBOX_CPU_CORE="${POKYD_DOSBOX_CPU_CORE:-dynamic}"
else
  DOSBOX_CPU_CORE="${POKYD_DOSBOX_CPU_CORE:-normal}"
fi

if [[ -z "$DOSBOX_X_BIN" ]]; then
  if command -v dosbox-x >/dev/null 2>&1; then
    DOSBOX_X_BIN="$(command -v dosbox-x)"
  fi
fi

if [[ -z "$DOSBOX_X_BIN" || ! -x "$DOSBOX_X_BIN" ]]; then
  echo "DOSBox-X not found. Install dosbox-x or set NOTES_DOSBOX_X to its full path."
  exit 1
fi

if [[ "$SKIP_BUILD" -eq 0 ]]; then
  echo "Building with Open Watcom (macOS host + DOSBox-X guest)..."
  "$ROOT_DIR/build.sh"
else
  echo "Skipping build (--no-build)."
fi

CONF_PATH="$(mktemp -t pokyd-run)"
trap 'rm -f "$CONF_PATH"' EXIT

# Build the pokyd.exe command line: keep the intro interactive unless explicitly skipped.
POKYD_CMD="pokyd.exe -consplit"
if [[ "$SKIP_INTRO" -eq 1 ]]; then
  POKYD_CMD="pokyd.exe -pokyd -consplit"
fi
if [[ -n "$POKYD_LLM_HOST" ]]; then
  POKYD_CMD="$POKYD_CMD -llm=$POKYD_LLM_HOST"
  echo "LLM mode: Pokyd will connect to bridge at $POKYD_LLM_HOST"
fi

AUTOEXEC_TAIL=$(cat <<EOF
if not exist C:\SLOVNIK.DAT if exist C:\assets\slova.pkd copy C:\assets\slova.pkd C:\SLOVA.PKD >nul
if not exist C:\SLOVNIK.DAT if exist C:\assets\pokydx.pkd copy C:\assets\pokydx.pkd C:\POKYDX.PKD >nul
if not exist C:\SLOVNIK.DAT if exist C:\slova.exe slova.exe
echo [pokyd] Starting pokyd.exe ...
REM -consplit keeps COMMAND.COM/autoexec lines above Pokyd; --skip-intro adds -pokyd
$POKYD_CMD
echo.
echo pokyd.exe ended with errorlevel %ERRORLEVEL%
echo Staying at C:\\ prompt --- type EXIT to close DOSBox.
EOF
)

if [[ "$EXIT_AFTER_POKYD" -eq 1 ]]; then
  AUTOEXEC_TAIL=$(cat <<EOF
$POKYD_CMD
exit
EOF
)
fi

# Build NE2000 networking config if LLM host is set.
# With slirp backend, the host machine is at 10.0.2.2 from inside the guest.
NE2000_SECTION=""
NE2000_AUTOEXEC=""
if [[ -n "$POKYD_LLM_HOST" ]]; then
  if [[ ! -f "$ROOT_DIR/assets/NE2000.COM" ]]; then
    echo "LLM mode needs assets/NE2000.COM (NE2000 packet driver for Watt-32)."
    echo "Download it: $ROOT_DIR/scripts/download-ne2000.sh"
    exit 1
  fi
  NE2000_SECTION=$(cat <<'NETEOF'
[ne2000]
ne2000=true
nicbase=0x300
nicirq=10
backend=slirp
NETEOF
)
  # Crynwr NE2000.COM: NE2000 [options] <packet_int_no> <int_level> <io_addr>
  # Must match [ne2000] nicirq/nicbase above; third arg must be 0x300 (hex), not 300 decimal.
  # Keep driver under assets/; avoid COPY /Y here (not all COMMAND.COM builds support /Y).
  # Never use SET VAR=C:\ with a trailing backslash before end-of-line: COMMAND.COM
  # treats \+newline as line continuation and appends the next line into the variable
  # (hence bogus paths like 0x300\wattcp.cfg). Use C: for "root of current drive".
  NE2000_AUTOEXEC=$(cat <<'NETEOF'
copy C:\assets\WATTCP.CFG C:\WATTCP.CFG >nul
SET WATTCP.CFG=C:
echo [pokyd] NE2000 assets\NE2000.COM 0x60 10 0x300
C:\assets\NE2000.COM 0x60 10 0x300
NETEOF
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
core=${DOSBOX_CPU_CORE}
# x87 FPU: required for sane Watcom float math; pair with dynamic core for LLM mode.
fpu=true
[dos]
ver=7.1
lfn=true
keyboardlayout=us
$NE2000_SECTION
[autoexec]
@echo off
echo [pokyd] Autoexec started
mount c "$ROOT_DIR"
c:
echo [pokyd] Switched to drive C
SET PATH=C:
$NE2000_AUTOEXEC
$AUTOEXEC_TAIL
EOF

echo
echo "Starting DOSBox-X..."
"$DOSBOX_X_BIN" -conf "$CONF_PATH"
