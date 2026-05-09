# Pokyd - pokyny pro AI / vyvojare

Tento repozitar je **MS-DOS program v C**, preklada se **Open Watcom** jako jedna jednotka ze `src/pokyd.c` (ostatni `src/*.c` se **vkladaji pres `#include`**).

## Kde hledat pravidla

| Tema | Soubor |
|------|--------|
| Jednotka prekladu, ceske nazvy funkci, rozsirovani | [.cursor/rules/pokyd-architecture.mdc](.cursor/rules/pokyd-architecture.mdc) |
| Font, `INT 10h`, `textmode`, ceska hlavicka | [.cursor/rules/pokyd-dos-int10-font.mdc](.cursor/rules/pokyd-dos-int10-font.mdc) |
| Watcom, `REGPACK`, stack INTRO, `-consplit` | [.cursor/rules/pokyd-watcom-dos.mdc](.cursor/rules/pokyd-watcom-dos.mdc) |

## Povinne konvence pri nove funkcionalite

1. **Nazvy funkci a maker cesky** (styl `VRAT_*`, `NASTAV_*`, `ZAPIS_*`), konzistentne s existujicim kodem.
2. **Po zmene video rezimu** (`textmode`, `INT 10h AX=0003`) znovu nastavit font podle `font` (`NASTAVSPRAVNYFONT`), pokud jde o vypis zavisly na `VRATDIAKRITIKU` nebo vlastnich glyfech.
3. Nove verejne funkce: deklarace do [`src/pokyd.h`](src/pokyd.h) a spravne poradi `#include` fragmentu v [`src/pokyd.c`](src/pokyd.c).
4. Kratky komentar u rutiny (ucel, uskali v DOSBox-X / Watcom).
5. **Komentare a dokumentace jen v ASCII** (bez hacku/carek v textu); UTF-8 soubory.

## Build

- `./build.sh` / `build.bat` -> `pokyd.exe`
- `./build-and-run.sh` - vyzaduje DOSBox-X (viz [README.md](README.md))

## Hromadne komentare ve zdrojacich

Skript [`scripts/pokyd_add_fn_comments.py`](scripts/pokyd_add_fn_comments.py) doplni pred funkce v `pokyd_v0.c`, `pokyd_v1.c`, `pokyd_v2.c`, `pokyd_na.c` obecne jednoradkove komentare (sablona). U verejnych nebo kritickych rutin popis rucne upresni. Spusteni:

`python3 scripts/pokyd_add_fn_comments.py`

Dvojite spusteni prida dalsi radky - pred opakovanim obnov soubor z git.
