# Pokyd (MS-DOS, Open Watcom)

Pokyd is a classic Czech conversational MS-DOS application originally developed in Borland C++ era code. This repository is modernized for Windows-hosted builds with Open Watcom and execution in DOSBox-X, and now also includes a macOS flow using DOSBox-X.

## Project Layout

- `src/` - all C sources, headers, and generated include fragments used for compilation
- `assets/` - static runtime content (text/data files, plugin data files, legacy resources)
- `scripts/` - DOSBox helper scripts
- `pokyd.exe` - built output in repository root

Runtime-generated/writable files remain in root (hybrid layout), such as `POKYD.CFG`, `KYDY/`, and logs.

## Prerequisites

### Windows flow

- Windows (PowerShell available)
- Open Watcom toolchain bundled in `watcom/`
- DOSBox-X (auto-detected or auto-downloaded by script)

### macOS flow

- macOS with Bash available
- DOSBox-X installed (`dosbox-x` available on PATH, or set `NOTES_DOSBOX_X`)
- Open Watcom DOS-capable tree in repo-local `watcom/` (same layout expected by Windows flow)

## Build and Run

### Windows

From repository root:

```bat
build-and-run.bat
```

This command:
1. Builds `pokyd.exe` from `src/pokyd.c` using Open Watcom.
2. Starts DOSBox-X and runs `pokyd.exe`.

You can also build only:

```bat
build.bat
```

Or run only (after build):

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1
```

### macOS

From repository root:

```bash
./build-and-run.sh
```

This command:
1. Builds `pokyd.exe` and `slova.exe` with Open Watcom host tools (auto-detected from `WATCOM` or `watcom/`).
2. Starts DOSBox-X and runs `pokyd.exe`.

You can also build only:

```bash
./build.sh
```

Optional:

```bash
./build-and-run.sh --exit-after-pokyd
```

This closes DOSBox-X automatically after `pokyd.exe` exits.

## Czech Text and DOS Font

Pokyd uses Czech text assets and custom rendering logic for DOS text mode. The app uses internal font switching routines (`NASTAVPOKYDFONT`, `NASTAVSPRAVNYFONT`) to preserve Czech Latin glyph readability under DOS video modes.

Important notes:
- Keep text assets in DOS-compatible encoding expected by the original codebase.
- Run in DOSBox text modes compatible with BIOS font switching.
- Avoid converting legacy text assets to UTF-8 without verifying in-DOS rendering.

After any BIOS mode set that reloads the ROM font (`INT 10h` mode change / `textmode`), custom glyphs must be reapplied before drawing the header or other text that relies on `VRATDIAKRITIKU()` - see `AGENTS.md` and `.cursor/rules/pokyd-dos-int10-font.mdc`.

## Developer documentation

- **[docs/prd.md](docs/prd.md)** - product requirements (scope, constraints, acceptance criteria)—read this before larger changes.
- **[docs/CODEMAP.md](docs/CODEMAP.md)** - unity-build include order and which source fragment owns which subsystem (saves time locating functions).
- **[AGENTS.md](AGENTS.md)** - how the repo is structured for AI/tools: unity build, Czech naming, where to add features.
- **[.cursor/rules/](.cursor/rules/)** - Cursor rules: architecture, DOS/font caveats, Watcom quirks, and a workspace rule to read the PRD/code map.
- **`scripts/pokyd_add_fn_comments.py`** - optional helper to prepend placeholder `/* ... */` lines before functions in large fragments (run from repo root; review/commit sensibly).

## Asset Resolution Behavior

The loader uses hybrid lookup:
- First tries root-relative path (for writable runtime files and backward compatibility).
- Falls back to `assets\...` for static files if not found in root.

This keeps legacy behavior while enabling a cleaner repository structure.
