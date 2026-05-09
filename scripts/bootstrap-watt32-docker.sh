#!/usr/bin/env bash
# Rebuild vendor/watt32-dos from Watt-32 sources using Docker (linux/amd64).
# Prerequisites: Docker, network access (GitHub + Ubuntu mirrors).
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENDOR_SRC="$ROOT_DIR/vendor/watt-32"
OUT_DIR="$ROOT_DIR/vendor/watt32-dos"

echo "==> Cloning Watt-32 into vendor/watt-32 (shallow, reproducible)..."
mkdir -p "$ROOT_DIR/vendor"
rm -rf "$VENDOR_SRC"
git clone --depth 1 https://github.com/gvanem/Watt-32.git "$VENDOR_SRC"

echo "==> Building wattcpwl.lib inside Docker (Ubuntu + Open Watcom snapshot + DOSBox)..."
docker run --rm --platform linux/amd64 \
  -e "SDL_VIDEODRIVER=dummy" \
  -v "$VENDOR_SRC:/watt" \
  ubuntu:22.04 bash -c '
set -ex
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -qq -y wget xz-utils dosbox ca-certificates coreutils git >/dev/null

cd /tmp
wget -q https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/ow-snapshot.tar.xz
mkdir -p /opt/watcom
tar xf /tmp/ow-snapshot.tar.xz -C /opt/watcom
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl:$PATH

cd /watt/util
wmake -h -f errnos.mak wc_err.exe

mkdir -p /watt/src/build/watcom/large /watt/inc/sys /watt/src/build/watcom
(timeout 90 dosbox -exit -c "mount c /watt" -c "c:" -c "cd util" -c "wc_err.exe -s > ..\\src\\build\\watcom\\syserr.c" -c "exit" || true)
(timeout 90 dosbox -exit -c "mount c /watt" -c "c:" -c "cd util" -c "wc_err.exe -e > ..\\inc\\sys\\watcom.err" -c "exit" || true)

cd /watt/src
../util/linux/mkmake -w -o watcom_l.mak -d build/watcom/large makefile.all WATCOM LARGE
../util/linux/mkdep -s.o -p\$\(OBJDIR\)/ *.c *.h > build/watcom/watt32.dep
echo "neterr.c: build/watcom/syserr.c" >> build/watcom/watt32.dep
wmake -h -f watcom_l.mak
test -f /watt/lib/wattcpwl.lib
echo "Watt-32 build OK."
'

echo "==> Installing headers + libs into vendor/watt32-dos ..."
mkdir -p "$OUT_DIR/lib"
rm -rf "$OUT_DIR/inc"
cp -a "$VENDOR_SRC/inc" "$OUT_DIR/"
cp -f "$VENDOR_SRC/lib/wattcpwl.lib" "$OUT_DIR/lib/wattcpwl.lib"
cp -f "$OUT_DIR/lib/wattcpwl.lib" "$OUT_DIR/lib/wattcplf.lib"

echo "Done. Run ./build.sh from the repo root (WATT_ROOT defaults to vendor/watt32-dos)."
