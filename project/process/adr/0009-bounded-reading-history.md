# ADR-0009: Bounded reading history (1 000 per sensor)

**Status:** Accepted (with the user, 2026-10-13) · **Date:** 2026-10-13

## Context
Every `read()` appends a `Reading` to `Sensor::readings_`. In a long-running system the history
would grow without limit, and the design gives no retention rule.

## Decision
Keep at most 1 000 readings per sensor, enforced in `read()`; when full, the oldest reading is
dropped. The limit is a named constant in `src/`.

## Consequences
- Memory stays bounded and predictable.
- Older readings are lost. "View sensor values" shows only the recent history.
- The constant would need to change if the team wants a different limit; a configurable limit
  would need an attribute that isn't drawn.
- **Open for the team:** whether 1 000 is the right size.
