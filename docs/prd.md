# Product Requirements Document: Pokyd Modernization

## 1. Product Overview

Pokyd is a legacy Czech-language MS-DOS conversational application. The goal of this modernization is to keep original behavior while making the repository maintainable and reproducible on modern hosts: **Windows**, **macOS**, and **Linux** (host Open Watcom + DOSBox-X for the DOS binary).

## 2. Objectives

- Reorganize repository structure:
  - source code in `src/`
  - static data/assets in `assets/`
  - built executable `pokyd.exe` in repository root
- Keep runtime compatibility in DOS/DOSBox.
- Provide one-step build and run workflows on Windows (`build.bat`) and Unix (`build.sh` on macOS/Linux).
- Document architecture, workflow, and legacy constraints.

## 3. In Scope

- Source relocation from root/plugin directories to `src/`.
- Static asset relocation to `assets/`.
- Runtime path resolver updates for hybrid root/assets lookup.
- Build/run scripting for Open Watcom + DOSBox-X on Windows, macOS, and Linux.
- GitHub Actions release workflow (tag push) building on `ubuntu-latest` and publishing `dist/pokyd-<tag>-dos.zip`.
- Documentation updates (`README.md`, `docs/prd.md`).

## 4. Out of Scope

- Full functional rewrite or language migration.
- Gameplay/chat logic redesign.
- UTF-8 localization overhaul of legacy text assets.

## 5. Users and Use Cases

### Primary users
- Retro software maintainers.
- DOS hobbyists and archivists.

### Core use cases
- Build Pokyd on Windows, macOS, or Linux.
- Run Pokyd inside DOSBox.
- Modify sources/assets without breaking runtime file loading.

## 6. Functional Requirements

1. **Build output**
   - `build.bat` (Windows) and `build.sh` (macOS/Linux) must compile the project from `src/`.
   - `build.sh` must select the correct host Open Watcom `wcl` for the current OS/CPU (see README host-dir table).
   - Output executable must be `pokyd.exe` in repo root.

2. **Run flow**
   - `build-and-run.bat` / `build-and-run.sh` must build and then launch DOSBox-X.
   - DOSBox runner must execute `pokyd.exe`.

3. **Asset loading**
   - Runtime path construction must support:
     - root-relative file access for writable outputs
     - fallback to `assets\` for static files

4. **Repository hygiene**
   - `.gitignore` must exclude generated binaries, objects, logs, and runtime outputs.

5. **Documentation**
   - README must include setup/build/run and structure.
   - PRD (`docs/prd.md`) must describe requirements, constraints, and acceptance criteria.
   - **CODEMAP** (`docs/CODEMAP.md`) documents unity-build fragment order and where to find major subsystems - maintain it when fragment responsibilities shift.

## 7. Non-Functional Requirements

- Preserve behavior of legacy DOS runtime logic.
- Maintain DOS-compatible file path and text processing assumptions.
- Keep scripts platform-appropriate: batch + PowerShell on Windows; POSIX shell on macOS/Linux.

## 8. Czech Language and Font Constraint

Pokyd uses Czech text content and depends on DOS-era Latin glyph behavior. The app must continue using its font handling routines (`NASTAVPOKYDFONT` and `NASTAVSPRAVNYFONT`) to display Czech text correctly in text mode.

Requirements:
- Keep legacy text file encodings compatible with DOS runtime.
- Validate visible Czech diacritics in DOSBox after build.
- Do not force UTF-8 conversion of legacy assets unless validated end-to-end.

## 9. Acceptance Criteria

- `build-and-run.bat` or `build-and-run.sh` successfully builds and launches `pokyd.exe` in DOSBox-X.
- Source files are under `src/`; static assets are under `assets/`.
- Key static files load correctly from `assets/` when absent in root.
- Writable runtime files are created/updated in root as before.
- README and PRD are present and accurate.

## 10. Validation Checklist

- Build passes with Open Watcom on Windows (`build.bat`) and on macOS/Linux (`build.sh` + `scripts/install-open-watcom.sh` or manual `WATCOM`).
- DOSBox starts and runs `pokyd.exe`.
- Config and conversation write paths remain writable in root.
- Czech text/font rendering appears correct in DOS text mode.

---

## 11. Optional Feature: Remote LLM Mode (scope extension)

This feature is **opt-in** and does not affect any existing behaviour when the
`-llm` flag is absent.  It is implemented via the `pokyd_llm.c` fragment and a
companion Node.js service (`bridge/`).

### Scope

- A new CLI flag `-llm=<host>:<port>` routes every user sentence to a remote
  Node.js TCP bridge instead of the legacy rule engine.
- Spontaneous lines (idle timer, jokes/weather, welcome, samomluva, insult cheat,
  dual-PC banter, resume, goodbye) use `INITIATIVE <kind>` on the same TCP session
  when LLM mode is active; legacy `VTIPY.TXT` / `SLOVNIK.DAT` paths run only if
  the bridge fails.
- The bridge runs an OpenAI agentic loop (tool execution included) and returns
  a plain-ASCII reply that is displayed through the existing `ODPOVED()` long-
  message path (`pozodp=100`, `dlouhe[]`).
- If the bridge is unreachable the program prints a message and falls back to
  the legacy rule engine for that turn.

### New files

| File | Role |
|------|------|
| `src/pokyd_llm.c` | DOS TCP client fragment (Watt-32, conditionally compiled) |
| `bridge/server.js` | Node.js TCP server + OpenAI agentic loop |
| `bridge/package.json` | Node dependencies |
| `bridge/.env.example` | Config template (API key, port, model) |
| `bridge/README.md` | Wire protocol spec + Watt-32 build guide |
| `assets/WATTCP.CFG` | Watt-32 network config template (DOSBox-X slirp + real HW) |
| `vendor/watt32-dos/` | Bundled Watt-32 `inc/` + Open Watcom large-model `wattcpwl.lib` |
| `assets/NE2000.COM` | NE2000 packet driver (Crynwr collection; see `assets/NE2000.LICENSE.txt`) |
| `scripts/install-open-watcom.sh` | Download ow-snapshot into `./watcom` (macOS/Linux; same as CI) |
| `scripts/bootstrap-watt32-docker.sh` | Rebuild `vendor/watt32-dos` via Docker on macOS/Linux |
| `scripts/package-release.sh` | Build + stage flat DOS layout + `dist/pokyd-<version>-dos.zip` |
| `scripts/push-release-tag.sh` | Push a git tag alone so GitHub Actions release webhook fires |
| `scripts/download-ne2000.sh` | Refetch `assets/NE2000.COM` from Internet Archive |
| `.github/workflows/release.yml` | Tag-triggered CI: Watcom, Watt-32, package, GitHub Release asset |

### Build requirements (LLM mode only)

- By default, `build.sh` sets `WATT_ROOT` to **`vendor/watt32-dos`** when `inc/tcp.h`
  is present (bundled tree - no separate Watt install needed).
- Override with `WATT_ROOT=/path/to/watt-32` if you maintain your own build.
- Without headers + library, `build.sh` builds Pokyd without LLM TCP code (same as before).
- Optional at compile time (same variable names on Windows and Unix): `POKYD_LLM_IP`,
  `POKYD_LLM_PORT` / `BRIDGE_PORT` bake default `llm_host` / `llm_port` into `pokyd.exe`;
  runtime `-llm=host:port` still overrides. See `scripts/pokyd-llm-env.sh`.

### DOS runtime requirements (LLM mode only)

- A packet driver for the DOS NIC loaded before `pokyd.exe`.
- `WATTCP.CFG` present (see `assets/WATTCP.CFG`).
- Node.js bridge running on a reachable host (`node bridge/server.js`).

### Encoding

- User input: Czech diacritics are already stripped to ASCII base letters by
  the existing `NAPIS()` routine; no encoding change on the DOS side.
- Bridge response: UTF-8 from OpenAI is transliterated to 7-bit ASCII by the
  bridge before being sent to DOS (`toAscii()` in `bridge/server.js`).

### Validation checklist (LLM mode)

- `pokyd.exe -pokyd -llm=10.0.2.2:8765` connects to the bridge in DOSBox-X.
- User input is forwarded and the OpenAI response is displayed correctly.
- Killing the bridge mid-session shows an error and the legacy engine resumes.
- Build without `WATT_ROOT` produces an identical `pokyd.exe` with no LLM code.
