# Pokyd source map (finding code quickly)

Use this file **before** grepping blindly across `src/`. The codebase is a **single translation unit**: only `src/pokyd.c` is passed to the compiler; every other `src/pokyd_*.c` is `#include`d in a **fixed order**. Symbols live in the fragment where they are defined unless duplicated by mistake.

## 1. Unity build include order

Exact sequence in `src/pokyd.c`:

| Order | File | Role |
|------:|------|------|
| 1 | `pokyd_pr.c` | Globals, types, shared buffers (`dlouhe[]`, conversation state). No substantial logic - depends on nothing later. |
| 2 | `pokyd_za.c` | Beginner hints (`ZACATECNIK`, `VYNULUJ_ZACATECNIK`). |
| 3 | `pokyd_v0.c` | Screen shell, header line, commands, `VRATDATA`, interrupts, input line, word memory. Czech header + diacritics path. |
| 4 | `pokyd_v1.c` | Paragraph output, `HELPF1`, intro imagery helpers, main conversation (`ODPOVED`), plugin UI (`PIS`, ...). |
| 5 | `pokyd_v2.c` | Load `SLOVNIK.DAT` (`NACTI_INTELIGENCI`), **INTRO** flow, external programs, misc system actions. |
| 6 | `pokyd_v3.c` | Low-level **INT 10h** output: colors, cursor, `NAPISRETEZEC`, decorative INTRO lines (`INTRO_CARY`). |
| 7 | `pokyd_fn.c` | VGA text font (`NASTAVPOKYDFONT`, `NASTAVSPRAVNYFONT`, `VRATDIAKRITIKU`, `ZMENZNAKY`). |
| 8 | `pokyd_na.c` | Fullscreen **settings UI** (`NASTAV`), mouse/keys, form pages (`S1`/`S2`). |
| 9 | `pokyd_sl.c` | IQ dictionary **condition parser** (`PRECTIPODMINKU`, `CTIPODMINKY`, `inteligence[][]`). |
| 10 | `pokyd_llm.c` | **Optional** remote LLM mode: Watt-32 TCP client (`LLM_INIT`, `LLM_CONNECT`, `LLM_SEND_RECV`, `LLM_CLOSE`). Compiled only when `POKYD_LLM_WATT` is defined (set `WATT_ROOT`). Stubs are present otherwise. |

**Headers:** `src/pokyd.h` holds declarations for public routines from fragments; check here first for the correct Czech name and `void`/parameters.

**Bootstrap:** `src/pokyd.c` itself implements DOS helpers (disk, scroll, video RAM), `main`, and pull all fragments together.

## 2. What to open for common tasks

| Task | Primary fragment(s) |
|------|---------------------|
| Intro screen, "Kontrola nastaveni", loading dictionary | `pokyd_v2.c` |
| Settings check arrows / INTRO labels wired to UI | `pokyd_na.c`, `pokyd_v2.c` |
| Czech header, command bar, `VRATDATA` strings | `pokyd_v0.c` |
| Font after mode change, diacritics | `pokyd_fn.c`, rules in `.cursor/rules/pokyd-dos-int10-font.mdc` |
| Anything drawn via BIOS / `NAPISRETEZEC` | `pokyd_v3.c` |
| Conversation replies, help F1 | `pokyd_v1.c` |
| IQ conditions / "inteligence" file semantics | `pokyd_sl.c` |
| Shared globals / buffers | `pokyd_pr.c` |
| Remote LLM TCP client (`-llm` flag, Watt-32) | `pokyd_llm.c`, `pokyd_pr.c` (llm_* globals), `pokyd.h` |

## 3. How to search effectively

1. **`rg '^void SOMEWORD'` or `rg SOMEWORD src/pokyd.h`** - Czech names are uppercase with underscores; declarations are in `pokyd.h`.
2. **`rg 'INTRO_' src`** - intro-related symbols cluster in `v2`/`v3`/`na`.
3. **`scripts/pokyd_add_fn_comments.py`** - adds one-line stub comments before functions in large files (see `AGENTS.md`); useful to see routine boundaries after running once.

Do **not** assume a symbol is in the "most logical" file without checking - follow the table above or `pokyd.h`.

## 4. VGA text attributes vs `PIS` / `INTRO_NAPIS`

In 80x25 color text, **`INT 10h` AH=09** uses **BL as the full attribute byte** (bits 4-7 background, 0-3 foreground - plus blink/intensity depending on mode). **`PIS(x,y,text, barva)`** passes `barva` through **unchanged**, so if callers pass **`13`** intending "foreground 13" only, the **upper nibble can be zero** and some setups show **wrong glyphs** next to rows drawn with **`INTRO_NAPIS`**, which merges **`(CTIBARVU() & 0xF0) | fg`** and preserves the brown intro background. For intro value columns that must match "muz"/"zena", sample **`CTIBARVU()`** at **`(23, NAST_Y+2)`** first - see **`INTRO()`** in `pokyd_v2.c` and `.cursor/rules/pokyd-dos-int10-font.mdc`.

## 5. Related docs

- **[prd.md](prd.md)** - scope, constraints, acceptance criteria (read first for product context).
- **[AGENTS.md](../AGENTS.md)** - AI/developer conventions for this repo.
- **[README.md](../README.md)** - build/run and layout.
- **[.cursor/rules/pokyd-dos-int10-font.mdc](../.cursor/rules/pokyd-dos-int10-font.mdc)** - font reload, `VRATDIAKRITIKU`, intro attribute merge details.
