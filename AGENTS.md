# Pokyd - guide for AI / developers

This repository is an **MS-DOS program in C**, compiled with **Open Watcom** as a single translation unit from `src/pokyd.c` (every other `src/*.c` is **pulled in via `#include`**).

## Read these first

| Priority | File | Purpose |
|----------|------|---------|
| 1 | [docs/prd.md](docs/prd.md) | Project scope, limits, how font the used, acceptance criteria |
| 2 | [docs/CODEMAP.md](docs/CODEMAP.md) | `#include` order of fragments, where to look for functions (no blind grepping) |

## Where to find rules

| Topic | File |
|-------|------|
| Translation unit, Czech function names, extending the project | [.cursor/rules/pokyd-architecture.mdc](.cursor/rules/pokyd-architecture.mdc) |
| Font, `INT 10h`, `textmode`, Czech header | [.cursor/rules/pokyd-dos-int10-font.mdc](.cursor/rules/pokyd-dos-int10-font.mdc) |
| Watcom, `REGPACK`, INTRO stack, `-consplit` | [.cursor/rules/pokyd-watcom-dos.mdc](.cursor/rules/pokyd-watcom-dos.mdc) |

## Required conventions when adding new functionality

1. **Czech function and macro names** (style `VRAT_*`, `NASTAV_*`, `ZAPIS_*`), consistent with the existing code.
2. **After a video mode change** (`textmode`, `INT 10h AX=0003`) reload the font according to `font` (`NASTAVSPRAVNYFONT`) whenever the output relies on `VRATDIAKRITIKU` or custom glyphs.
3. New public functions: declare them in [`src/pokyd.h`](src/pokyd.h) and use the correct `#include` order in [`src/pokyd.c`](src/pokyd.c).
4. Short comment above each routine (purpose, gotchas in DOSBox-X / Watcom).
5. **Comments and documentation in ASCII only** (no Czech diacritics in text); files saved as UTF-8.

## Build

- `./build.sh` / `build.bat` -> `pokyd.exe`
- `./build-and-run.sh` - requires DOSBox-X (see [README.md](README.md))
- LLM networking env (same on Windows and Unix): `BRIDGE_PORT`, `POKYD_LLM_HOST`, `POKYD_LLM_IP`, `POKYD_LLM_PORT` - resolved in `scripts/pokyd-llm-env.sh` / `pokyd-llm-env.ps1`

## Bulk comments in source files

The script [`scripts/pokyd_add_fn_comments.py`](scripts/pokyd_add_fn_comments.py) inserts generic single-line comments (template) above functions in `pokyd_v0.c`, `pokyd_v1.c`, `pokyd_v2.c`, `pokyd_na.c`. For public or critical routines refine the description by hand. Run with:

`python3 scripts/pokyd_add_fn_comments.py`

Running it twice adds extra lines - restore the file from git before re-running.
