# Pokyd LLM

Pokyd je česká konverzační aplikace pro MS-DOS od **KýblSoftu**; tento repozitář jej rozšiřuje o napojení na LLM (OpenAI) prostřednictvím NodeJS bridge. 

Původní zdrojové kódy jsou [zde](https://old.kyblsoft.cz/iqpokyd)

Původní [README z roku 2002](./docs/pokyd_old.md)

![Pokyd screenshot](./docs/pokyd_scr01.png)
![Pokyd screenshot2](./docs/pokyd_scr02.png)


## Struktura repozitáře

- `src/` – veškeré C zdrojáky, hlavičky a další fragmenty
- `assets/` – statická data (slovníky, doplňky)
- `scripts/` – pomocné skripty (DOSBox-X, build, Watt-32, release; macOS / Linux / Windows)
- `bridge/` – Node.js server (LLM), viz [bridge/README.md](bridge/README.md)
- `POKYDLLM.BAT` – spuštění `pokyd.exe` s LLM uvnitř DOSBoxu (po `build-and-run-llm`)

---

## Co a jak nainstalovat

Program se původně kompiloval v DOSu v Borland C++, teď je možno jej kompilovat i na dnešních systémech s využitím Open Watcom.

- **Open Watcom v2** (překladač pro DOS) – Windows: `watcom\binnt\`; macOS/Linux: `./scripts/install-open-watcom.sh` nebo [releases](https://github.com/open-watcom/open-watcom-v2/releases).
- **DOSBox-X** (DOS emulátor) - do složky dosbox. [DOSBox-X](https://dosbox-x.com/)
- Pro **LLM režim** navíc: **Node.js 18+**, API klíč a .env proměnné v `bridge/` (podrobnosti v [bridge/README.md](bridge/README.md)).


### macOS a Linux

Na obou platformách se používají stejné skripty v kořeni repa: **`build.sh`**, **`build-and-run.sh`**, **`build-and-run-llm.sh`**. Překlad probíhá na hostiteli (nikoliv v DOSu) přes Open Watcom; výstup je **`pokyd.exe`** pro DOS / DOSBox-X.

1. **DOSBox-X**  
   - **macOS:** doporučeno [Homebrew](https://brew.sh/) – `brew install dosbox-x`  
   - **Linux:** balíček z distribuce, nebo build z [DOSBox-X](https://dosbox-x.com/)  
   - Pokud `dosbox-x` není v `PATH`, nastav **`NOTES_DOSBOX_X`** na plnou cestu k binárce.

2. **Open Watcom**  
   - Jednorázově stáhni snapshot (stejný jako v CI):  
     `./scripts/install-open-watcom.sh`  
     (rozbalí do **`./watcom`** v kořeni repozitáře)  
   - Nebo ručně z [Open Watcom v2 releases](https://github.com/open-watcom/open-watcom-v2/releases) (archiv `.tar.xz`) do `~/watcom`, `/opt/watcom`, nebo `./watcom`.  
   - `build.sh` hledá instalaci v pořadí: **`WATCOM`**, `./watcom`, `~/watcom`, `/usr/local/watcom`, `/opt/watcom`.  
   - Snapshot obsahuje překladače pro všechny platformy; **`build.sh` vybere správný hostitelský `wcl`** podle OS a CPU:

   | Hostitel | Adresář s `wcl` |
   |----------|------------------|
   | Linux x86_64 | `binl64`, pak `binl` |
   | Linux arm64 | `arml64`, pak `binl64` |
   | macOS Apple Silicon | `armo64`, pak `arml64` |
   | macOS Intel | `binl64`, pak `binl` |

   - Příklad: `export WATCOM=/cesta/k/watcom`

3. **Node.js** (jen pro LLM)  
   - macOS: `brew install node` nebo [nodejs.org](https://nodejs.org/)  
   - Linux: balíček `nodejs` / nvm / nodesource  
   - V `bridge/`: `npm install`, zkopíruj `bridge/.env.example` → `bridge/.env`

4. **Watt-32** (jen pro LLM)  
   - `build.sh` automaticky použije **`vendor/watt32-dos`**, pokud existuje `inc/tcp.h` a knihovna v `lib/`.  
   - Vygenerování (Docker na macOS i Linuxu, stejný skript):  
     `./scripts/bootstrap-watt32-docker.sh`  
     (Windows: `scripts\bootstrap-watt32-docker.bat`)

5. **Spuštění „vše v jednom“ s LLM**  
   - Po bootstrapu Watt-32 a doplnění `bridge/.env`:  
     `./build-and-run-llm.sh`

### Windows

1. **Open Watcom**  
   - Z [releases](https://github.com/open-watcom/open-watcom-v2/releases) stáhni snapshot pro **Windows** (instalátor nebo ZIP podle nabídky).  
   - Rozbal obsah tak, aby existovala cesta  
     **`pokyd\watcom\binnt\wcl.exe`**  
   - `build.bat` očekává přesně tuto strukturu

2. **DOSBox-X**  
   - **Bez LLM:** stačí běžná VS instalace (např. `dosbox\x64\Release\dosbox-x.exe` nebo portable VS build).  
   - **S LLM:** potřebuješ **MinGW win64** build se **slirp** backendem (NE2000). Visual Studio build v `dosbox\` síť neumí — skript `run-dosbox.ps1` / `build-and-run-llm.bat` stáhne MinGW automaticky do **`.tools\dosbox-x-mingw\`**, pokud tam ještě není.  
   - Volitelně nastav cestu k MinGW `dosbox-x.exe` do **`NOTES_DOSBOX_X`** (musí podporovat slirp).

3. **Node.js** (jen pro LLM bridge na stejném PC)  
   - Instalace z [nodejs.org](https://nodejs.org/) (LTS). V adresáři `bridge` spusť `npm install`, zkopíruj`.env.example` do souboru `.env` a uprav parametry – viz [bridge/README.md](bridge/README.md).

4. **Watt-32** (jen pro LLM)  
   - Stejně jako na macOS/Linux: `build.bat` automaticky zapne LLM režim, pokud existuje **`vendor\watt32-dos`** s hlavičkami a knihovnou (např. `inc\tcp.h`, `lib\wattcplf.lib` nebo `lib\wattcpwl.lib`).  
   - Adresář **`vendor\watt32-dos`** vygeneruj jednorázově přes Docker:  
     `scripts\bootstrap-watt32-docker.bat`  

5. **Spuštění „vše v jednom“ s LLM (Windows)**  
   - Po bootstrapu Watt-32 a doplnění `bridge\.env` spusť:  
     `build-and-run-llm.bat`


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

LLM režim (Watt-32 + Node most + NE2000 v DOSBox-X) – vše najednou:

```bat
build-and-run-llm.bat
```

**Znovu spustit Pokyd s LLM uvnitř DOSBoxu** (po ukončení hry, na výzvě `C:\>`):

- Bridge na hostiteli musí stále běžet (spouští ho `build-and-run-llm.bat`, nebo ručně `node bridge\server.js`).
- V DOSBoxu zadej:

```text
POKYDLLM
```

Soubor [`POKYDLLM.BAT`](POKYDLLM.BAT) v kořeni repozitáře (v DOSu `C:\POKYDLLM.BAT`) spustí `pokyd.exe -consplit -llm 10.0.2.2 8765`. Samotné `pokyd.exe` LLM nezapne – je potřeba přepínač `-llm`. Pokud máš jiný port než 8765, uprav v batch souboru řádky `POKYD_LLM_IP` / `POKYD_LLM_PORT` (stejně jako `BRIDGE_PORT` v `bridge\.env`).

Podrobnosti k síti, `WATTCP.CFG` a `assets\NE2000.COM`: [bridge/README.md](bridge/README.md).

### macOS a Linux

Z kořene repozitáře (stejné příkazy na obou OS):

| Účel | Příkaz |
|------|--------|
| Překlad + DOSBox | `./build-and-run.sh` |
| Jen překlad | `./build.sh` |
| LLM (Watt-32 + bridge + DOSBox) | `./build-and-run-llm.sh` |
| Release zip (lokálně) | `./scripts/package-release.sh 1.0` |

`build-and-run.sh` sestaví `pokyd.exe` a `slova.exe` a spustí DOSBox-X.

---

## Pomocné skripty (`scripts/`)

| Skript | macOS / Linux | Windows alt. |
|--------|---------------|---------|
| `install-open-watcom.sh` | stáhne OW do `./watcom` | — (`watcom\binnt\wcl.exe`) |
| `bootstrap-watt32-docker.sh` | Watt-32 → `vendor/watt32-dos` | `bootstrap-watt32-docker.bat` |
| `package-release.sh` | zip pro DOS | — |
| `push-release-tag.sh` | push tagu → CI release | — |
| `download-ne2000.sh` | stíhne ovladač do `assets/NE2000.COM` | — |
| `pokyd-llm-env.sh` | sdílené proměnné LLM (source z `build.sh`) | `pokyd-llm-env.ps1` |

DOSBox na Windows: `run-dosbox.ps1`, `build-and-run-llm.ps1` (viz `scripts/common.ps1`).

---

## Konfigurovatelné proměnné

Konfigurační soubor hry je v souboru `POKYD.CFG`. V LLM módu jej hra odesílá na server při každém dotazu.

| Proměnná | Výchozí | Účel |
|----------|---------|----------------|
| `BRIDGE_PORT` | `8765` (nebo z `bridge/.env`) | Port nodejs middleware co posílá LLM data |
| `POKYD_LLM_IP` | `10.0.2.2` | Adresa bridge **z pohledu DOS hosta**  |
| `POKYD_LLM_PORT` | jako `BRIDGE_PORT` | TCP port, na který se `pokyd.exe` připojí (`-llm=`) |
| `POKYD_LLM_HOST` | odvozené `IP:port` | Argument `-llm=host:port` (má prioritu nad `IP`+`PORT`) |
| `POKYD_DOSBOX_CPU_CORE` | `dynamic` při LLM | Nastavení DOSBox-X `[cpu] core=` |
| `NOTES_DOSBOX_X` | — | Plná cesta k `dosbox-x` / `dosbox-x.exe` |

**Při sestavení** (`build.sh` / `build.bat` s Watt-32): pokud je nastaveno `POKYD_LLM_IP` a/nebo `POKYD_LLM_PORT` (nebo `BRIDGE_PORT` na Windows), zapíší se výchozí hodnoty do `pokyd.exe` (přepíše je runtime `-llm=host:port`).

**Příklady:**

```bash
# macOS/Linux – vlastní port bridge i Pokyd
export BRIDGE_PORT=9000
export POKYD_LLM_IP=10.0.2.2
./build-and-run-llm.sh
```

```bat
rem Windows – stejné názvy
set BRIDGE_PORT=9000
set POKYD_LLM_IP=10.0.2.2
build-and-run-llm.bat
```

```bash
# Jen spuštění DOSBox s LLM (bridge už běží na hostu)
POKYD_LLM_HOST=10.0.2.2:8765 ./build-and-run.sh
```

Podrobnosti k protokolu a Watt-32: [bridge/README.md](bridge/README.md).

---

## Dokumentace pro AI agenty

- **[docs/prd.md](docs/prd.md)** – požadavky, rozsah, kritéria akceptace  
- **[docs/CODEMAP.md](docs/CODEMAP.md)** – pořadí `#include` fragmentů a mapa subsystémů  
- **[AGENTS.md](AGENTS.md)** – rozcestník
- **[bridge/README.md](bridge/README.md)** – protokol TCP, Node bridge, OpenAI, `.env`  
- **[.cursor/rules/](.cursor/rules/)** – pravidla pro architekturu, DOS/font, Watcom 
- **`scripts/pokyd_add_fn_comments.py`** – pomůcka pro komentáře u funkcí

---
