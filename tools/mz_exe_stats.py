#!/usr/bin/env python3
"""Quick MZ (.exe) stats + entry disassembly for comparing DOS builds (no IDA required)."""

from __future__ import annotations

import argparse
import struct
import sys

try:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_16
except ImportError:
    print("pip install capstone", file=sys.stderr)
    raise


def mz_load(path: str) -> tuple[bytes, int]:
    with open(path, "rb") as f:
        d = f.read()
    if len(d) < 32 or d[:2] != b"MZ":
        raise ValueError(f"{path}: not an MZ executable")
    e_cparhdr = struct.unpack_from("<H", d, 8)[0]
    e_ip = struct.unpack_from("<H", d, 0x14)[0]
    e_cs = struct.unpack_from("<H", d, 0x16)[0]
    hdr = e_cparhdr * 16
    entry_off = hdr + (e_cs << 4) + e_ip
    if entry_off >= len(d):
        raise ValueError(f"{path}: bad entry offset {entry_off:#x}")
    return d, entry_off


def byte_stats(data: bytes) -> dict[str, int]:
    return {
        "int10": data.count(bytes([0xCD, 0x10])),
        "int21": data.count(bytes([0xCD, 0x21])),
        "int16": data.count(bytes([0xCD, 0x16])),
    }


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("exes", nargs="+", help="paths to MZ executables")
    ap.add_argument("-n", "--bytes", type=int, default=96, help="disasm bytes at entry")
    args = ap.parse_args()

    md = Cs(CS_ARCH_X86, CS_MODE_16)
    for path in args.exes:
        data, eo = mz_load(path)
        st = byte_stats(data)
        print(f"=== {path}")
        print(f"    size={len(data)} entry_off=0x{eo:x} int10={st['int10']} int21={st['int21']} int16={st['int16']}")
        chunk = data[eo : eo + args.bytes]
        for ins in md.disasm(chunk, eo):
            print(f"    {ins.address:08x}: {ins.mnemonic:8} {ins.op_str}")
        print()


if __name__ == "__main__":
    main()
