# Plan: implement the cabin monitor (2026-10-13)

The spec is [2026-10-13-cabin-monitor-specification.md](../specs/2026-10-13-cabin-monitor-specification.md).

## Steps

**Inputs**
- [x] Re-parse the current drawings with the verifier readers. The C1, C2, C3 and G1 fixes are
      confirmed: 77 class elements, 14 state elements, 7 calls.
- [x] Write the spec in `specs/`.

**Toolchain**
- [x] Install MSYS2 UCRT64 with gcc, cmake and ninja. The result is GCC 16.2.
- [x] Work around libclang 18 failing to parse GCC 16 `<memory>`/`<chrono>`:
  - install MSYS2 `clang-libs` (libclang 23);
  - run the verifier with `LIBCLANG_LIBRARY_PATH=C:/msys64/ucrt64/bin`;
  - cross-check: every example report regenerates byte-identical.

**Code**
- [x] Headers in `include/cabin/`, one per class, plus `date_time.hpp` and `hardware.hpp`.
- [x] Sources in `src/`, plus `reading_history.hpp` (internal) and `demo.cpp`.
- [x] `scenarios/check_sensors.cpp`.
- [x] `CMakeLists.txt`, with `CABIN_BUILD_TESTS` and `CABIN_COVERAGE` (both OFF by default).

**Tests**
- [x] Unit tests: doctest v2.4.11 vendored, 47 cases and 147 assertions, all pass.
- [x] Coverage with gcovr: lines 97 %, decisions 97 %, raw branches 70 %. The misses are explained in the report.

**Verification**
- [x] Run the verifier:
  - state 100 %;
  - sequence 100 %;
  - class 93.5 %: 5 *Changed*, all caused by the verifier's `T[]` normalization defect (see the
    report);
  - 0 build warnings.

**Records**
- [x] ADRs:
  - 0010 (alert delivery) and 0011 (friend list revised) added;
  - 0009 accepted;
  - 0007 superseded;
  - index fixed.
- [x] Final report: `research/2026-10-13-cabin-monitor-implementation-report.md`.

## Open, outside this work's authority

- [ ] Class report at 100 %. Blocked by the verifier, because `tools/` is read-only for this work.
  - `elements.ntype()` does not map `T[]` to `list<T>`, although
    [UML-CPP-MAPPING.md](../../../tools/umlverify/docs/UML-CPP-MAPPING.md) (line 130) says it
    should.
  - **One-line fix:** in `ntype`, before the `vector|list|set` rule, add
    `t = re.sub(r"^(.+)\[\]$", r"list<\1>", t)`.
  - **Alternative fix:** the team draws the five return types as `vector<T>` instead of `T[]`.

This plan stays in `plans/` and does not move to `plans/completed/` until that item is closed,
because AGENTS.md requires 100 % first.
