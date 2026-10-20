# Plan — temperature alarm (completed 2026-10-20)

Each step ends with `python arduino/tools/fom.py "<what changed>"`, one row in
[`fom/history.csv`](../../../fom/history.csv).

- [x] Write the design as mermaid in `model/`; generate `diagrams/input/*.drawio` with
      `tools/generate_diagrams.py`; render and inspect every drawing. — *iteration 0, gap 100 %*
- [x] Headers in `impl/include/tempalarm/` exactly as designed; sources; hardware seam
      `hal.hpp` with a host simulation; demo; one scenario program per sequence diagram.
      Verifier: 100 % on all four reports. — *iteration 1, gap 0 %*
- [x] Copy the core into the sketch (`tools/sync_firmware.py`), write `hal_arduino.cpp` and
      the wiring-only `.ino`; compile with arduino-cli for UNO and Mega, no warnings in our code.
      — *iteration 1, 9 702 B flash*
- [x] doctest unit tests (one file per class, every state-machine arrow) and benchmarks.
      — *iteration 2, 46/46*
- [x] Website: protocol, line splitting, WebSerial, simulated device, Dexie store, chart,
      view, composition root; `node --test` tests; checked in headless Edge at desktop and
      phone width, light and dark. — *iteration 3, 18/18*
- [x] Optimise the slowest benchmark: hand-written number parser instead of `strtod`
      (stricter, matches `protocol.js`). — *iteration 4, parse 307 → 11 ns, flash −236 B*
