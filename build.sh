#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/pokyd-llm-env.sh
. "$ROOT_DIR/scripts/pokyd-llm-env.sh"

if [[ ! -f "$ROOT_DIR/src/pokyd.c" ]]; then
  echo "Source file missing: src/pokyd.c"
  exit 1
fi

find_watcom_root() {
  local candidates=()
  if [[ -n "${WATCOM:-}" ]]; then
    candidates+=("$WATCOM")
  fi
  candidates+=(
    "$ROOT_DIR/watcom"
    "$HOME/watcom"
    "/usr/local/watcom"
    "/opt/watcom"
  )

  local cand
  for cand in "${candidates[@]}"; do
    if [[ -d "$cand" ]]; then
      if [[ -x "$cand/armo64/wcl" || -x "$cand/arml64/wcl" || -x "$cand/binl64/wcl" || -x "$cand/binl/wcl" || -x "$cand/binw/wcl.exe" ]]; then
        printf '%s\n' "$cand"
        return 0
      fi
    fi
  done
  return 1
}

WATCOM_ROOT="$(find_watcom_root || true)"
if [[ -z "$WATCOM_ROOT" ]]; then
  echo "Open Watcom not found."
  echo "Set WATCOM=/path/to/watcom or install it under $ROOT_DIR/watcom."
  exit 1
fi

WATCOM_HOST_BIN=""
for host_dir in armo64 arml64 binl64 binl; do
  if [[ -x "$WATCOM_ROOT/$host_dir/wcl" ]]; then
    WATCOM_HOST_BIN="$WATCOM_ROOT/$host_dir"
    break
  fi
done

if [[ -z "$WATCOM_HOST_BIN" ]]; then
  echo "No host Open Watcom compiler frontend found in $WATCOM_ROOT."
  echo "Expected one of: armo64/wcl, arml64/wcl, binl64/wcl, binl/wcl."
  exit 1
fi

export WATCOM="$WATCOM_ROOT"
export EDPATH="$WATCOM/eddat"
export INCLUDE="$WATCOM/h:$WATCOM/h/dos"
export LIB="$WATCOM/lib286:$WATCOM/lib286/dos"
export PATH="$WATCOM_HOST_BIN:$WATCOM/binw:$PATH"

# ---------------------------------------------------------------------------
# Optional: Watt-32 TCP/IP stack for the remote LLM mode (-llm=host:port).
# Set WATT_ROOT to the Watt-32 installation directory.  Expected layout:
#   $WATT_ROOT/inc/tcp.h          Watt-32 header
#   $WATT_ROOT/lib/wattcplf.lib   16-bit Open Watcom large-model library
# See bridge/README.md for build instructions and DOSBox-X NE2000 setup.
# ---------------------------------------------------------------------------
LLM_CFLAGS=""
LLM_LIBS=""
LLM_STACK="-k16384"
WATT_ROOT="${WATT_ROOT:-}"

# Bundled Watt-32 headers + libs (macOS-friendly; see scripts/bootstrap-watt32-docker.sh).
if [[ -z "$WATT_ROOT" && -f "$ROOT_DIR/vendor/watt32-dos/inc/tcp.h" ]]; then
  WATT_ROOT="$ROOT_DIR/vendor/watt32-dos"
fi

if [[ -n "$WATT_ROOT" ]]; then
  WATT_INC="$WATT_ROOT/inc"
  WATT_LIB=""
  if [[ -f "$WATT_ROOT/lib/wattcplf.lib" ]]; then
    WATT_LIB="$WATT_ROOT/lib/wattcplf.lib"
  elif [[ -f "$WATT_ROOT/lib/wattcpwl.lib" ]]; then
    WATT_LIB="$WATT_ROOT/lib/wattcpwl.lib"
  fi
  if [[ -f "$WATT_INC/tcp.h" && -n "$WATT_LIB" ]]; then
    LLM_CFLAGS="-DPOKYD_LLM_WATT=1 -I$WATT_INC"
    if pokyd_llm_configured; then
      _llm_ip="${POKYD_LLM_IP:-10.0.2.2}"
      _llm_port="${POKYD_LLM_PORT:-$(pokyd_read_bridge_port "$ROOT_DIR")}"
      LLM_CFLAGS="$LLM_CFLAGS -DPOKYD_LLM_DEFAULT_HOST=\\\"${_llm_ip}\\\" -DPOKYD_LLM_DEFAULT_PORT=${_llm_port}"
      echo "LLM compile defaults: ${_llm_ip}:${_llm_port} (runtime -llm=host:port overrides)"
    fi
    LLM_LIBS="$WATT_LIB"
    # Watt-32 static library pushes DGROUP near the 64KiB limit; shrink stack to leave room.
    # INTRO still fits if kept shallow; raise toward -k3584 only if link fits (E2020).
    # Watt + near data share DGROUP with stack (-k bytes). Over ~480B needs -k3008 not -k3584.
  # llm_sock lives on far heap so -k3008 still links after connect helpers grew.
    LLM_STACK="-k3008"
    echo "Watt-32 found at $WATT_ROOT -- LLM mode (-llm=host:port) will be compiled in."
  else
    echo "WARNING: WATT_ROOT=$WATT_ROOT set but tcp.h or wattcplf.lib not found -- skipping LLM."
    echo "  Expected: $WATT_INC/tcp.h and $WATT_LIB"
  fi
else
  echo "WATT_ROOT not set -- building without LLM networking (set WATT_ROOT to enable)."
fi

echo "Building with Open Watcom (host tools: $WATCOM_HOST_BIN)..."

rm -f "$ROOT_DIR/pokyd.exe" "$ROOT_DIR/slova.exe"
rm -f "$ROOT_DIR"/src/*.obj

pushd "$ROOT_DIR/src" >/dev/null
# -k: stack size (bytes). INTRO needs > default 2KiB, but DGROUP must stay <=64KiB (near/far data + bss).
# shellcheck disable=SC2086
wcl -bt=dos -ml -zm -zq -ox $LLM_STACK $LLM_CFLAGS -fo=. -fe=../pokyd.exe pokyd.c $LLM_LIBS
wcl -bt=dos -ml -zm -zq -ox -fo=. -fe=../slova.exe slova.c
popd >/dev/null

if [[ ! -f "$ROOT_DIR/pokyd.exe" ]]; then
  echo "Build finished but pokyd.exe was not created."
  exit 1
fi

echo "Build succeeded: pokyd.exe"
if [[ -f "$ROOT_DIR/slova.exe" ]]; then
  echo "Utility ready: slova.exe"
fi
