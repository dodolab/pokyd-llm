#!/usr/bin/env bash
# Build Pokyd with Watt-32 LLM support, start the Node OpenAI bridge on the host,
# then launch DOSBox-X with NE2000 slirp so the guest reaches the bridge at 10.0.2.2.
#
# Prerequisites: Open Watcom (see build.sh), Node 18+, DOSBox-X, bridge/.env with OPENAI_API_KEY,
# and `lsof` (standard on macOS) so we can free BRIDGE_PORT after a crash (avoids EADDRINUSE).
#
# Usage:
#   ./build-and-run-gpt.sh
#   ./build-and-run-gpt.sh --exit-after-pokyd   # CI/automation only (closes DOSBox when pokyd exits)
#
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BRIDGE_DIR="$ROOT_DIR/bridge"
DOSBOX_X_BIN="${NOTES_DOSBOX_X:-}"
BRIDGE_PID=""
BRIDGE_PORT="${BRIDGE_PORT:-8765}"

# Optional: read BRIDGE_PORT from bridge/.env (do not source whole file).
if [[ -f "$BRIDGE_DIR/.env" ]]; then
  line="$(grep -E '^BRIDGE_PORT=' "$BRIDGE_DIR/.env" | tail -1 || true)"
  if [[ -n "${line}" ]]; then
    v="${line#BRIDGE_PORT=}"
    v="${v%%#*}"
    v="$(echo "$v" | tr -d ' \t\r\"')"
    if [[ -n "$v" ]] && [[ "$v" =~ ^[0-9]+$ ]]; then
      BRIDGE_PORT="$v"
    fi
  fi
fi

if [[ -z "${BRIDGE_PORT}" || "${BRIDGE_PORT}" -lt 1 ]]; then
  BRIDGE_PORT=8765
fi

BRIDGE_PID_FILE="$BRIDGE_DIR/.bridge.pid"

# Kill any process still listening on TCP port (stale node after DOSBox crash / missed trap).
pokyd_free_tcp_port() {
  local port="$1"
  [[ -z "$port" ]] && return 0
  if ! command -v lsof >/dev/null 2>&1; then
    echo "[gpt] Warning: lsof not found - cannot auto-free port ${port}."
    return 0
  fi
  local p
  for p in $(lsof -t "-iTCP:${port}" -sTCP:LISTEN 2>/dev/null); do
    echo "[gpt] Releasing TCP ${port} - sending SIGTERM to pid ${p}..."
    kill -TERM "$p" 2>/dev/null || true
  done
  sleep 0.4
  for p in $(lsof -t "-iTCP:${port}" -sTCP:LISTEN 2>/dev/null); do
    echo "[gpt] Port ${port} still busy - SIGKILL pid ${p}..."
    kill -KILL "$p" 2>/dev/null || true
  done
}

cleanup() {
  set +e
  if [[ -n "${BRIDGE_PID}" ]] && kill -0 "${BRIDGE_PID}" 2>/dev/null; then
    echo "[gpt] Stopping bridge (pid ${BRIDGE_PID})..."
    kill -TERM "${BRIDGE_PID}" 2>/dev/null || true
    wait "${BRIDGE_PID}" 2>/dev/null || true
  fi
  pokyd_free_tcp_port "${BRIDGE_PORT}"
  rm -f "${BRIDGE_PID_FILE}"
  set -e
}
trap cleanup EXIT INT TERM HUP

if ! command -v node >/dev/null 2>&1; then
  echo "node not found. Install Node.js 18+."
  exit 1
fi

if [[ -z "$DOSBOX_X_BIN" ]]; then
  if command -v dosbox-x >/dev/null 2>&1; then
    DOSBOX_X_BIN="$(command -v dosbox-x)"
  fi
fi
if [[ -z "$DOSBOX_X_BIN" || ! -x "$DOSBOX_X_BIN" ]]; then
  echo "DOSBox-X not found. Install dosbox-x or set NOTES_DOSBOX_X."
  exit 1
fi

if [[ ! -f "$BRIDGE_DIR/.env" ]]; then
  echo "Missing bridge/.env - copy bridge/.env.example and set OPENAI_API_KEY."
  exit 1
fi

if ! grep -qE '^OPENAI_API_KEY=.+' "$BRIDGE_DIR/.env"; then
  echo "bridge/.env must set OPENAI_API_KEY=... (non-empty)."
  exit 1
fi

if [[ ! -f "$ROOT_DIR/vendor/watt32-dos/inc/tcp.h" ]]; then
  echo "Watt-32 bundle missing: vendor/watt32-dos/inc/tcp.h"
  echo "Run ./scripts/bootstrap-watt32-docker.sh or clone Watt-32 into vendor/watt32-dos."
  exit 1
fi

echo "[gpt] Installing bridge npm dependencies (if needed)..."
(cd "$BRIDGE_DIR" && npm install --silent)

echo "[gpt] Ensuring TCP port ${BRIDGE_PORT} is free (previous bridge / crash)..."
pokyd_free_tcp_port "${BRIDGE_PORT}"

echo "[gpt] Starting OpenAI bridge on port ${BRIDGE_PORT} (bind from bridge/.env)..."
(cd "$BRIDGE_DIR" && node server.js >>"$ROOT_DIR/bridge/pokyd-bridge.log" 2>&1) &
BRIDGE_PID=$!
echo "${BRIDGE_PID}" >"${BRIDGE_PID_FILE}"
sleep 1
if ! kill -0 "${BRIDGE_PID}" 2>/dev/null; then
  echo "[gpt] Bridge failed to start. Last lines of bridge/pokyd-bridge.log:"
  tail -20 "$ROOT_DIR/bridge/pokyd-bridge.log" 2>/dev/null || true
  exit 1
fi

echo "[gpt] Bridge pid=${BRIDGE_PID} (log: bridge/pokyd-bridge.log)"
echo "[gpt] Building pokyd.exe with LLM (Watt-32)..."
"$ROOT_DIR/build.sh"

# DOSBox-X slirp: guest uses 10.0.2.2 to reach services on the host.
export POKYD_LLM_HOST="10.0.2.2:${BRIDGE_PORT}"
echo "[gpt] Launching DOSBox-X with -llm=${POKYD_LLM_HOST}"
# Do not `exec` — we need the EXIT trap to stop the bridge when DOSBox-X closes.
"$ROOT_DIR/build-and-run.sh" --no-build "$@"
