# ADR-0005: `checkSensors()` fires `MeasurementIntervalElapsed` and `AlertSent`

**Status:** Accepted · **Date:** 2026-10-13

## Context
The state machine moves Monitoring → Alerting user → Monitoring through the events
`MeasurementIntervalElapsed` and `AlertSent`. No drawn class owns a timer or sends notifications,
and `User` has no operation for receiving an alert. If nothing fires `AlertSent`, the cabin stays
in *Alerting user* forever.

## Decision
- One call to `checkSensors()` is one measurement interval.
- It reads every sensor, sets `lastCheck_` to whether any sensor was out of range, and fires
  `MeasurementIntervalElapsed`.
- If any alerts were produced, it then fires `AlertSent` and returns the alerts.
- `addSensor()` fires `addSensor`. `removeSensor()` fires `LastSensorRemoved` when `noSensors()`
  holds.

## Consequences
- No undrawn class or method is needed, and the state and sequence reports can reach 100 %.
- *Alerting user* lasts only for the moment between the two events.
- The events are calls a class makes to itself, so they don't add sequence messages.
