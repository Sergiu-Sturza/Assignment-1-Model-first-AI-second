# ADR-0006: Sensor values and time come from a free-function hardware seam

**Status:** Accepted · **Date:** 2026-10-09

## Context
`read()` takes no input, and readings and alerts need a timestamp. Scenarios must be
deterministic: no clock, no randomness. Any added class would count as an Extra in the class
report.

## Decision
`include/cabin/hardware.hpp` declares free functions only:
- `sample(sensorId)` and `now()`;
- `setSampler()` and `setClock()` to replace them;
- `reset()` to restore the defaults.

The default sampler is a fixed simulated source. The default clock starts at a fixed epoch and
advances one minute per call.

## Consequences
- No extra classes. Free functions are never sequence messages.
- Tests and scenarios control their inputs exactly.
- The seam is global state: every test must call `reset()`, and it is not thread-safe. This is
  acceptable because the scenarios are single-threaded.
- Real hardware would plug in through `setSampler()` without touching the drawn classes.
