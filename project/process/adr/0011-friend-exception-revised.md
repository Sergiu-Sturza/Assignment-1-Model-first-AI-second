# ADR-0011: Friend exception, revised to cover the sensor subclasses

**Status:** Accepted · **Date:** 2026-10-13 · **Supersedes:** [ADR-0007](0007-friend-exception.md)

## Context
ADR-0007 made `Reading` and `Sensor` befriend the classes on the drawn `checkSensors()` path, so
that the traced scenario makes exactly the 7 drawn calls. Implementing it showed that ADR-0007
missed two cases:

1. **`Sensor` → subclasses.** Each subclass's `read()` needs the sensor's id to sample it.
   - `id` is drawn `- id: int`, which is private to `Sensor`.
   - The trace names a call by the class that *defines* the method. So
     `TemperatureSensor::read()` calling the inherited `Sensor::getId()` is recorded as an extra
     message `TemperatureSensor → Sensor : getId()`.
   - Making `id_` protected would instead show as a *Changed* visibility in the class report.
2. **`TemperatureSensor` → `Cabin`.** To tell a *high* temperature alert from a *low* one (the
   activity diagram), `checkSensors()` must compare the value with `minThreshold`. That attribute
   is drawn private, and there is no getter.

## Decision
Exactly these friendships, and no others:

| Class | Declares friends |
|---|---|
| `Reading` | `TemperatureSensor`, `PowerMeter`, `SecuritySensor`, `Cabin` |
| `Sensor` | `TemperatureSensor`, `PowerMeter`, `SecuritySensor`, `Cabin` |
| `TemperatureSensor` | `Cabin` |

- Code on the drawn call path reads fields directly.
- Code outside it uses the public getters: `addSensor`, `removeSensor`, `User`, the demo and the
  tests.

## Consequences
- The sequence report is at 100 %; the class report is unaffected, because friend declarations
  are invisible to it.
- Encapsulation is weakened in three classes, in one documented and reviewable place each.
- If the team draws getter calls in the sequence diagram, or adds a protected accessor to the
  class diagram, this ADR can be superseded and the friendships removed.
