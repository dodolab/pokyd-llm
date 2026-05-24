#!/usr/bin/env bash
# Runs inside Linux container (mounted as /watt). Invoked by bootstrap-watt32-docker.{sh,bat}.
set -ex
export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -qq -y wget xz-utils dosbox ca-certificates coreutils git findutils sed >/dev/null

# Windows host git clone often stores CRLF; mkmake then emits a truncated watcom_l.mak
# whose default target is doxygen (needs Windows hhc.exe). Strip CR before configure.
find /watt/src -type f \( -name 'makefile.all' -o -name '*.mak' \) -exec sed -i 's/\r$//' {} +

cd /tmp
wget -q https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/ow-snapshot.tar.xz
mkdir -p /opt/watcom
tar xf /tmp/ow-snapshot.tar.xz -C /opt/watcom
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl:$PATH

cd /watt/util
wmake -h -f errnos.mak wc_err.exe

mkdir -p /watt/src/build/watcom/large /watt/inc/sys /watt/src/build/watcom
export SDL_VIDEODRIVER=dummy
timeout 120 dosbox -exit -c "mount c /watt" -c "c:" -c "cd util" \
  -c "wc_err.exe -s > ..\src\build\watcom\syserr.c" \
  -c "wc_err.exe -e > ..\watcom.err" \
  -c "exit"
# ..\inc\sys\watcom.err fails on Linux mounts; DOS may write WATCOM.ERR / SYSERR.C instead.
werr="$(find /watt -maxdepth 1 -iname 'watcom.err' | head -1)"
test -n "$werr"
mv -f "$werr" /watt/inc/sys/watcom.err
test -s /watt/inc/sys/watcom.err
serr="$(find /watt/src/build/watcom -maxdepth 1 -iname 'syserr.c' | head -1)"
test -n "$serr"
[ "$serr" = "/watt/src/build/watcom/syserr.c" ] || mv -f "$serr" /watt/src/build/watcom/syserr.c
test -s /watt/src/build/watcom/syserr.c

cd /watt/src
../util/linux/mkmake -w -o watcom_l.mak -d build/watcom/large makefile.all WATCOM LARGE
../util/linux/mkdep -s.o -p\$\(OBJDIR\)/ *.c *.h > build/watcom/watt32.dep
echo "neterr.c: build/watcom/syserr.c" >> build/watcom/watt32.dep
wmake -h -f watcom_l.mak all
test -f /watt/lib/wattcpwl.lib
echo "Watt-32 build OK."
