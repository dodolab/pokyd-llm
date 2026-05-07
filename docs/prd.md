# Product Requirements Document: Pokyd Modernization

## 1. Product Overview

Pokyd is a legacy Czech-language MS-DOS conversational application. The goal of this modernization is to keep original behavior while making the repository maintainable and reproducible on modern Windows hosts.

## 2. Objectives

- Reorganize repository structure:
  - source code in `src/`
  - static data/assets in `assets/`
  - built executable `pokyd.exe` in repository root
- Keep runtime compatibility in DOS/DOSBox.
- Provide one-step Windows build and run workflow.
- Document architecture, workflow, and legacy constraints.

## 3. In Scope

- Source relocation from root/plugin directories to `src/`.
- Static asset relocation to `assets/`.
- Runtime path resolver updates for hybrid root/assets lookup.
- Build/run scripting for Open Watcom + DOSBox-X on Windows.
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
- Build Pokyd on Windows.
- Run Pokyd inside DOSBox.
- Modify sources/assets without breaking runtime file loading.

## 6. Functional Requirements

1. **Build output**
   - `build.bat` must compile project from `src/`.
   - Output executable must be `pokyd.exe` in repo root.

2. **Run flow**
   - `build-and-run.bat` must build and then launch DOSBox runner.
   - DOSBox runner must execute `pokyd.exe`.

3. **Asset loading**
   - Runtime path construction must support:
     - root-relative file access for writable outputs
     - fallback to `assets\` for static files

4. **Repository hygiene**
   - `.gitignore` must exclude generated binaries, objects, logs, and runtime outputs.

5. **Documentation**
   - README must include setup/build/run and structure.
   - PRD must describe requirements, constraints, and acceptance criteria.

## 7. Non-Functional Requirements

- Preserve behavior of legacy DOS runtime logic.
- Maintain DOS-compatible file path and text processing assumptions.
- Keep scripts Windows-friendly (batch + PowerShell).

## 8. Czech Language and Font Constraint

Pokyd uses Czech text content and depends on DOS-era Latin glyph behavior. The app must continue using its font handling routines (`NASTAVPOKYDFONT` and `NASTAVSPRAVNYFONT`) to display Czech text correctly in text mode.

Requirements:
- Keep legacy text file encodings compatible with DOS runtime.
- Validate visible Czech diacritics in DOSBox after build.
- Do not force UTF-8 conversion of legacy assets unless validated end-to-end.

## 9. Acceptance Criteria

- `build-and-run.bat` successfully builds and launches `pokyd.exe` in DOSBox.
- Source files are under `src/`; static assets are under `assets/`.
- Key static files load correctly from `assets/` when absent in root.
- Writable runtime files are created/updated in root as before.
- README and PRD are present and accurate.

## 10. Validation Checklist

- Build passes with Open Watcom on Windows.
- DOSBox starts and runs `pokyd.exe`.
- Config and conversation write paths remain writable in root.
- Czech text/font rendering appears correct in DOS text mode.
