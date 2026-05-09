#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

echo "Building with Open Watcom (host tools: $WATCOM_HOST_BIN)..."

rm -f "$ROOT_DIR/pokyd.exe" "$ROOT_DIR/slova.exe"
rm -f "$ROOT_DIR"/src/*.obj

pushd "$ROOT_DIR/src" >/dev/null
# -k: stack size (bytes). INTRO needs > default 2KiB, but DGROUP must stay ?64KiB (near/far data + bss).
wcl -bt=dos -ml -zm -zq -ox -k16384 -fo=. -fe=../pokyd.exe pokyd.c
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
