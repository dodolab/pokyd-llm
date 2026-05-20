# Pokyd LLM

Pokyd je klasická česká konverzační aplikace pro MS-DOS. Původní program pochází od **KýblSoft**; tento repozitář ho modernizuje (struktura zdrojáků, build skripty, dokumentace) a rozšiřuje ho o **režim LLM**: DOSová část se přes TCP spojí s **mostem v Node.js** (`bridge/server.js`), který propojuje Pokyd s rozhraním OpenAI. Bez mostu a bez sítě zůstává k dispozici původní rule-based chování.

## Rozložení repozitáře

- `src/` – veškeré C zdrojáky, hlavičky a fragmenty pro jednotkový překlad
- `assets/` – statická data (slovníky, doplňky, starší zdroje)
- `scripts/` – pomocné skripty pro DOSBox-X
- `bridge/` – Node.js server (LLM), viz [bridge/README.md](bridge/README.md)
- `pokyd.exe` – výstup překladu v kořeni projektu

Do kořene zapisuje i běh aplikace (`POKYD.CFG`, `KYDY/`, logy apod.).

---

## Co je potřeba nainstalovat

### Společné

- **Open Watcom v2** (překladač pro hostitele i cíl DOS). Oficiální buildy: [Open Watcom v2 – Releases](https://github.com/open-watcom/open-watcom-v2/releases) (např. soubor typu `ow-snapshot` pro vaši platformu).
- **DOSBox-X** (doporučeno kvůli spolehlivému mountu složky a textovému režimu). [DOSBox-X](https://dosbox-x.com/) – na Windows může skript stáhnout portable verzi do `.tools/`, pokud není nikde v PATH.
- Pro **LLM režim** navíc: **Node.js 18+**, klíč API a nastavení v `bridge/` (podrobnosti v [bridge/README.md](bridge/README.md)).

Složka `watcom/` v kořeni repozitáře je v `.gitignore` – toolchain si musíte stáhnout a rozbalit sami (viz níže).

### macOS

1. **DOSBox-X**  
   - Doporučeno: [Homebrew](https://brew.sh/) – `brew install dosbox-x`  
   - Nebo instalátor z webu; pokud binárka není v PATH, nastavte proměnnou `NOTES_DOSBOX_X` na plnou cestu k spustitelnému souboru `dosbox-x`.

2. **Open Watcom**  
   - Stáhněte snapshot pro macOS (archiv `.tar.xz` z výše uvedených releases).  
   - Rozbalte například do **`/Users/vás/watcom`** nebo přímo do **`/cesta/k/pokyd/watcom`**.  
   - Skript `build.sh` hledá překladač v tomto pořadí: proměnné **`WATCOM`**, pak **`./watcom`**, `~/watcom`, `/usr/local/watcom`, `/opt/watcom`. Uvnitř musí být například `binl64/wcl` nebo `binl/wcl` (záleží na architektuře).  
   - Po rozbalení můžete exportovat např.:  
     `export WATCOM=/cesta/k/watcom`

3. **Node.js** (jen pro LLM most)  
   - např. `brew install node` nebo [nodejs.org](https://nodejs.org/).

4. **Watt-32** (volitelné – jen pokud chcete překládat Pokyd s parametrem **`-llm=host:port`** a TCP stackem ve DOSu)  
   - Skript `build.sh` automaticky použije **`vendor/watt32-dos`**, pokud tam jsou hlavičky a knihovna (např. `inc/tcp.h`, `lib/wattcplf.lib`).  
   - Tyto soubory repozitář standardně neobsahuje; můžete je vygenerovat přes Docker:  
     `./scripts/bootstrap-watt32-docker.sh`  
     (potřebujete **Docker Desktop** nebo jiný Docker; skript stáhne zdroje Watt-32 a zbuildí knihovnu pro Open Watcom large model).  
   - Alternativa: vlastní instalace Watt-32 a nastavení **`WATT_ROOT`** na kořen té instalace (musí sedět rozložení souborů popsané v komentářích v `build.sh`).  
   - Bez `WATT_ROOT` / bez `vendor/watt32-dos` se projekt přeloží **bez** síťového LLM režimu – pro základní hraní v DOSBoxu to stačí.

5. **Spuštění „vše v jednom“ s LLM (macOS/Linux)**  
   - Po splnění výše uvedeného (včetně `bridge/.env`):  
     `./build-and-run-gpt.sh`  
   - Podrobnosti opět v [bridge/README.md](bridge/README.md).

### Windows

1. **Open Watcom**  
   - Z [releases](https://github.com/open-watcom/open-watcom-v2/releases) stáhněte snapshot pro **Windows** (instalátor nebo ZIP podle nabídky).  
   - Nainstalujte nebo rozbalte obsah tak, aby existovala cesta  
     **`pokyd\watcom\binnt\wcl.exe`**  
     (tj. kořen Watcomu = podsložka `watcom` v kořeni klonu repozitáře).  
   - `build.bat` očekává přesně tuto strukturu (`set WATCOM=%~dp0watcom`).

2. **DOSBox-X**  
   - Nainstalujte globálně, nebo nechte skript `scripts\run-dosbox.ps1` stáhnout portable verzi do **`.tools\dosbox-x\`** (složka je ignorována gitem).  
   - Volitelně nastavte **`NOTES_DOSBOX_X`** na plnou cestu k `dosbox-x.exe`.

3. **PowerShell**  
   - Součást moderních Windows; skripty předpokládají možnost spustit  
     `powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1`.

4. **Node.js** (jen pro LLM most na stejném PC)  
   - Instalace z [nodejs.org](https://nodejs.org/) (LTS). V adresáři `bridge`: `npm install`, kopie `.env.example` → `.env`, úprava klíčů – viz [bridge/README.md](bridge/README.md).

5. **Watt-32 na Windows**  
   - Výchozí `build.bat` překládá **bez** volitelného Watt-32 LLM rozšíření.  
   - Plná integrace TCP/LLM na Windows by vyžadovala ruční rozšíření příkazu `wcl` obdobně jako v `build.sh` (include knihoven Watt-32); pro LLM workflow je v dokumentaci k projektu veden především tok **macOS/Linux + `build.sh` + bridge**.

---

## Sestavení a spuštění

### Windows

Z kořene repozitáře:

```bat
build-and-run.bat
```

Postup:

1. Sestaví `pokyd.exe` ze `src/pokyd.c` pomocí Open Watcom (`watcom\binnt\wcl.exe`).
2. Spustí DOSBox-X a naběhne `pokyd.exe`.

Jen překlad:

```bat
build.bat
```

Jen DOSBox (po úspěšném buildu):

```powershell
powershell -ExecutionPolicy Bypass -File scripts\run-dosbox.ps1
```

### macOS

Z kořene repozitáře:

```bash
./build-and-run.sh
```

Postup:

1. Sestaví `pokyd.exe` a `slova.exe` pomocí hostitelských nástrojů Open Watcom (viz výše).
2. Spustí DOSBox-X a `pokyd.exe`.

Jen překlad:

```bash
./build.sh
```

Volitelně ukončit DOSBox po ukončení Pokyd:

```bash
./build-and-run.sh --exit-after-pokyd
```

Režim s připojením na most (hostitel a port podle vaší sítě):

```bash
POKYD_LLM_HOST=10.0.2.2:8765 ./build-and-run.sh
```

(nejdřív musí běžet Node most – viz `bridge/README.md`.)

---

## Český text a font v DOSu

Pokyd používá česká data a vlastní práci s textovým režimem a fontem (`NASTAVPOKYDFONT`, `NASTAVSPRAVNYFONT`). Textové soubory nechte v kódování, které očekává původní kód; po změně video režimu (`INT 10h`, `textmode`) je potřeba znovu nastavit font podle dokumentace v `AGENTS.md` a `.cursor/rules/pokyd-dos-int10-font.mdc`.

---

## Dokumentace pro vývojáře

- **[docs/prd.md](docs/prd.md)** – požadavky, rozsah, kritéria akceptace  
- **[docs/CODEMAP.md](docs/CODEMAP.md)** – pořadí `#include` fragmentů a mapa subsystémů  
- **[AGENTS.md](AGENTS.md)** – jednotkový překlad, české názvy funkcí, kam přidávat změny  
- **[bridge/README.md](bridge/README.md)** – protokol TCP, Node most, OpenAI, `.env`  
- **[.cursor/rules/](.cursor/rules/)** – pravidla pro architekturu, DOS/font, Watcom  
- **`scripts/pokyd_add_fn_comments.py`** – volitelná pomůcka pro komentáře u funkcí ve velkých fragmentech  

---

## Hledání souborů (assets)

Načítání používá hybridní strategii: nejdřív relativně ke kořeni (zápis za běhu, zpětná kompatibilita), při chybě náhrada z `assets\...`.
