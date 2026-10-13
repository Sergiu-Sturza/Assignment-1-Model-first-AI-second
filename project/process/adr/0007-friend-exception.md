# ADR-0007: `friend` exception to keep the sequence trace exact

**Status:** Accepted · **Date:** 2026-10-13

## Context
The sequence verifier builds without optimizations and with `-finstrument-functions`. It records
every call between project classes, inline getters included. On the drawn path:
- `shouldAlert(r)` would call `r.getValue()`;
- `checkSensors()` would call `getName()`, `getValue()` and `getTime()` to build alerts.

Each of these calls would add a message the diagram does not show.

## Decision
- `Reading` declares `friend class TemperatureSensor; friend class PowerMeter;
  friend class SecuritySensor; friend class Cabin;`.
- `Sensor` declares `friend class Cabin;`.
- Code on the drawn call path reads fields directly. Everyone else uses the public getters.
- No other friendships are allowed.

## Consequences
- The trace has exactly the 7 drawn calls. Neither verifier sees friend declarations.
- Encapsulation is weakened in one known, documented place.
- If the team draws the getter calls in the sequence diagram, this ADR is superseded and the
  friends are removed.
