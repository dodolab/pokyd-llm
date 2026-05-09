#!/usr/bin/env python3
"""
Insert one-line /* ... */ comments before top-level C functions in Pokyd fragments.
Optional overrides: edit DESC_ASCII (ASCII only for portability).
"""
import re
import sys
from pathlib import Path

# ASCII-only; extend when documenting new public APIs.
DESC_ASCII = {
    "VRATDIAKRITIKU": "Maps ASCII header letters to VGA font slots; needs NASTAVPOKYDFONT after mode set.",
    "NAPISHLAVICKOVYRADEK": "Draws top title bar (author line + KYBLSoft + version).",
    "NASTAVPOKYDFONT": "Loads Czech glyphs into VGA RAM (see pokyd_fn.c, zmeneneznaky).",
    "NACTI_INTELIGENCI": "Loads and decodes SLOVNIK.DAT into datasoubor and IQ blocks.",
}


def fn_name_from_line(line):
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    m = re.match(r"^void\s+interrupt\s+(\w+)\s*\(", line)
    if m:
        return m.group(1)
    m = re.match(
        r"^(?:static\s+)?(?:BYTE|WORD|DWORD|int|signed\s+char|signed\s+long|void(?:\s+far)?)\s+(?:interrupt\s+)?(\w+)\s*\(",
        line,
    )
    if m:
        return m.group(1)
    return None


def prev_is_comment(out_lines):
    for j in range(len(out_lines) - 1, -1, -1):
        s = out_lines[j].strip()
        if not s:
            continue
        return s.startswith("/*") or s.startswith("//")
    return False


def process(path: Path, dry_run: bool) -> int:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines(keepends=True)
    out = []
    inserted = 0
    for line in lines:
        name = fn_name_from_line(line)
        if (
            name
            and line.rstrip().endswith("{")
            and not prev_is_comment(out)
            and name not in ("if", "while", "for", "switch")
        ):
            desc = DESC_ASCII.get(
                name,
                "Rutina %s - viz implementace a nazvy promennych (konvence Pokyd)." % name,
            )
            out.append("/* %s */\n" % desc)
            inserted += 1
        out.append(line)
    new_text = "".join(out)
    if not dry_run and new_text != text:
        path.write_text(new_text, encoding="utf-8")
    print("%s: inserted %d comment block(s)" % (path, inserted))
    return inserted


if __name__ == "__main__":
    dry = "--dry" in sys.argv
    root = Path(__file__).resolve().parents[1]
    for name in ("pokyd_v0.c", "pokyd_v1.c", "pokyd_v2.c", "pokyd_na.c"):
        f = root / "src" / name
        if f.exists():
            process(f, dry_run=dry)
        else:
            print("missing", f, file=sys.stderr)
