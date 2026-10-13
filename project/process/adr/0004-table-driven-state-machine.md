# ADR-0004: Table-driven state machine inside `Cabin`

**Status:** Accepted · **Date:** 2026-10-13

## Context
`state-cabin.drawio` has 3 states, 4 events and 6 transitions. The verifier reads the state
machine back from a fixed C++ pattern (AGENTS.md rule 4,
[door.hpp](../../../examples/3-state-simple/impl/include/home/door.hpp)).

## Decision
Inside `Cabin`:
- `enum class State { Unconfigured, Monitoring, AlertingUser }`;
- `enum class Event { addSensor, LastSensorRemoved, MeasurementIntervalElapsed, AlertSent }`;
- the guards `hasSensors`, `outOfRange`, `inRange` as private `bool … const`;
- `static constexpr Transition transitions[]` with 6 rows, all five fields on every row;
- `State state_ = State::Unconfigured;`.

`handle(Event)` takes the first row that matches the current state and event and whose guard
holds. An event with no row is ignored. The `Deleted cabin` arrow to the end symbol needs no row.

## Consequences
- The drawing and the code have the same shape. A new transition is one new row.
- `handle()` does no allocation and runs in O(rows).
- No entry or exit actions, composite states or history, because the mapping has none.
