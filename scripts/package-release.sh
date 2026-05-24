#!/usr/bin/env bash
# Package an MS-DOS release zip: pokyd.exe, runtime data, assets/, POKYDLLM.BAT.
# Used locally and by .github/workflows/release.yml (tag pushes only).
#
# Usage:
#   ./scripts/package-release.sh [version-label]
#
# Env:
#   SKIP_BUILD=1     skip ./build.sh (pokyd.exe must already exist)
#   OUT_DIR=...      output directory (default: dist/)
#
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="${1:-${GITHUB_REF_NAME:-${RELEASE_VERSION:-dev}}}"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/dist}"
STAGE="$OUT_DIR/pokyd-${VERSION}"
ZIP="$OUT_DIR/pokyd-${VERSION}-dos.zip"

if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
  "$ROOT_DIR/build.sh"
fi

if [[ ! -f "$ROOT_DIR/pokyd.exe" ]]; then
  echo "Missing pokyd.exe (run build.sh first or unset SKIP_BUILD)."
  exit 1
fi

required_root=(
  POKYDLLM.BAT
  SLOVNIK.DAT
  POKYD.CFG
  WATTCP.CFG
)
for f in "${required_root[@]}"; do
  if [[ ! -f "$ROOT_DIR/$f" ]]; then
    echo "Missing required file: $f"
    exit 1
  fi
done

if [[ ! -d "$ROOT_DIR/assets" ]]; then
  echo "Missing assets/ directory."
  exit 1
fi

rm -rf "$STAGE"
mkdir -p "$STAGE/assets"

cp -f "$ROOT_DIR/pokyd.exe" "$STAGE/"
cp -f "$ROOT_DIR/POKYDLLM.BAT" "$STAGE/"
for f in "${required_root[@]}"; do
  cp -f "$ROOT_DIR/$f" "$STAGE/"
done

if [[ -f "$ROOT_DIR/PROFIL.PKD" ]]; then
  cp -f "$ROOT_DIR/PROFIL.PKD" "$STAGE/"
fi

cp -a "$ROOT_DIR/assets/." "$STAGE/assets/"

mkdir -p "$OUT_DIR"
rm -f "$ZIP"
(
  cd "$STAGE"
  if command -v zip >/dev/null 2>&1; then
    zip -rq9 "$ZIP" .
  else
    # GitHub Actions ubuntu-latest has zip; fallback for minimal images.
    python3 - "$ZIP" <<'PY'
import pathlib
import sys
import zipfile

zip_path = pathlib.Path(sys.argv[1]).resolve()
root = pathlib.Path(".").resolve()
with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED) as zf:
    for path in sorted(root.rglob("*")):
        if path.is_file():
            zf.write(path, path.relative_to(root).as_posix())
PY
  fi
)

echo "Release zip: $ZIP"
echo "Contents:"
find "$STAGE" -type f | sort | sed "s|^$STAGE/|  |"
