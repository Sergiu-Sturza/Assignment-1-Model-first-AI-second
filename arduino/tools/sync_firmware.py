#!/usr/bin/env python3
"""Copy the verified C++ core from arduino/impl/ into the Arduino sketch.

    python arduino/tools/sync_firmware.py           copy (run after every change in impl/)
    python arduino/tools/sync_firmware.py --check   exit 1 if the sketch's copy is stale

The Arduino IDE only compiles files inside the sketch folder, so the classes the verifier
checked are copied, byte for byte, into temperature_alarm/src/:

    impl/include/tempalarm/*.hpp  ->  temperature_alarm/src/tempalarm/
    impl/src/*.cpp, *.hpp         ->  temperature_alarm/src/

except the host-only files (the simulated board and the demo). The copy is never edited:
impl/ is the one source, the sketch adds only the wiring (.ino) and the hardware adapter
(hal_arduino.cpp).
"""
import argparse
import sys
from pathlib import Path

ARDUINO = Path(__file__).resolve().parents[1]
IMPL = ARDUINO / "impl"
SKETCH_SRC = ARDUINO / "temperature_alarm" / "src"
HOST_ONLY = {"hal_sim.hpp", "hal_host.cpp", "demo.cpp"}
NOTE = ("# Generated — do not edit\n\nA copy of `arduino/impl/`, made by "
        "`python arduino/tools/sync_firmware.py`. Edit the files there, then run it again.\n")


def wanted():
    """{destination: source} of every file the sketch needs."""
    files = {}
    for src in sorted((IMPL / "include" / "tempalarm").glob("*.hpp")):
        if src.name not in HOST_ONLY:
            files[SKETCH_SRC / "tempalarm" / src.name] = src
    for src in sorted((IMPL / "src").glob("*.[ch]pp")):
        if src.name not in HOST_ONLY:
            files[SKETCH_SRC / src.name] = src
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--check", action="store_true", help="only check that the copy is current")
    args = parser.parse_args()

    files = wanted()
    present = {p for p in SKETCH_SRC.rglob("*") if p.is_file() and p.name != "README.md"}
    stale = sorted(present - set(files))
    differ = sorted(d for d, s in files.items() if not d.exists() or d.read_bytes() != s.read_bytes())

    if args.check:
        for p in stale + differ:
            print(f"  out of date: {p.relative_to(ARDUINO).as_posix()}")
        if stale or differ:
            sys.exit("the sketch's copy of impl/ is stale: run python arduino/tools/sync_firmware.py")
        print(f"  the sketch's copy of impl/ is current ({len(files)} files)")
        return

    for p in stale:
        p.unlink()
    for dst in differ:
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(files[dst].read_bytes())
    (SKETCH_SRC / "README.md").write_text(NOTE, encoding="utf-8", newline="\n")
    print(f"  {len(files)} files in temperature_alarm/src/ ({len(differ)} updated, {len(stale)} removed)")


if __name__ == "__main__":
    main()
