# Pokyd (MS-DOS, Open Watcom)

Pokyd is a classic Czech conversational MS-DOS application originally developed in Borland C++ era code. This repository is modernized for Windows-hosted builds with Open Watcom and execution in DOSBox-X.

## Project Layout

- `src/` - all C sources, headers, and generated include fragments used for compilation
- `assets/` - static runtime content (text/data files, plugin data files, legacy resources)
- `scripts/` - DOSBox helper scripts
- `pokyd.exe` - built output in repository root

Runtime-generated/writable files remain in root (hybrid layout), such as `POKYD.CFG`, `KYDY/`, and logs.

## Prerequisites

- Windows (PowerShell available)
- Open Watcom toolchain bundled in `watcom/`
- DOSBox-X (auto-detected or auto-downloaded by script)

## Build and Run

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

## Czech Text and DOS Font

Pokyd uses Czech text assets and custom rendering logic for DOS text mode. The app uses internal font switching routines (`NASTAVPOKYDFONT`, `NASTAVSPRAVNYFONT`) to preserve Czech Latin glyph readability under DOS video modes.

Important notes:
- Keep text assets in DOS-compatible encoding expected by the original codebase.
- Run in DOSBox text modes compatible with BIOS font switching.
- Avoid converting legacy text assets to UTF-8 without verifying in-DOS rendering.

## Asset Resolution Behavior

The loader uses hybrid lookup:
- First tries root-relative path (for writable runtime files and backward compatibility).
- Falls back to `assets\...` for static files if not found in root.

This keeps legacy behavior while enabling a cleaner repository structure.
