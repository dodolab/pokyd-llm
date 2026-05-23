#!/usr/bin/env bash
# Shared LLM / bridge environment (source from build-and-run*.sh; do not execute directly).
#
# Cross-platform names (same on Windows PowerShell scripts):
#   BRIDGE_PORT      - TCP port for node bridge/server.js (host)
#   POKYD_LLM_HOST   - full host:port for pokyd.exe -llm= (guest view)
#   POKYD_LLM_IP     - host part only (default 10.0.2.2 for DOSBox-X slirp)
#   POKYD_LLM_PORT   - port part for Pokyd (defaults to BRIDGE_PORT, then 8765)
#   POKYD_DOSBOX_CPU_CORE - optional DOSBox [cpu] core when LLM is on

pokyd_bridge_port_default() {
  echo "8765"
}

# Read BRIDGE_PORT from env, then bridge/.env (does not source the whole .env file).
pokyd_read_bridge_port() {
  local root="${1:-}"
  local port="${BRIDGE_PORT:-}"

  if [[ -z "$port" && -n "$root" && -f "$root/bridge/.env" ]]; then
    local line
    line="$(grep -E '^BRIDGE_PORT=' "$root/bridge/.env" | tail -1 || true)"
    if [[ -n "$line" ]]; then
      port="${line#BRIDGE_PORT=}"
      port="${port%%#*}"
      port="$(echo "$port" | tr -d ' \t\r\"')"
    fi
  fi

  if [[ -z "$port" ]] || ! [[ "$port" =~ ^[0-9]+$ ]] || [[ "$port" -lt 1 ]]; then
    port="$(pokyd_bridge_port_default)"
  fi
  echo "$port"
}

# Build host:port for pokyd.exe -llm= from POKYD_LLM_* (POKYD_LLM_HOST wins if set).
pokyd_resolve_llm_host() {
  local root="${1:-}"

  if [[ -n "${POKYD_LLM_HOST:-}" ]]; then
    echo "$POKYD_LLM_HOST"
    return 0
  fi

  local ip="${POKYD_LLM_IP:-10.0.2.2}"
  local port="${POKYD_LLM_PORT:-}"
  if [[ -z "$port" ]]; then
    port="$(pokyd_read_bridge_port "$root")"
  fi
  echo "${ip}:${port}"
}

# True when any LLM-related override is set (enables -llm= in build-and-run.sh).
pokyd_llm_configured() {
  [[ -n "${POKYD_LLM_HOST:-}" || -n "${POKYD_LLM_IP:-}" || -n "${POKYD_LLM_PORT:-}" ]]
}
