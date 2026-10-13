# ADR-0010: Alerts reach a `User` through `addAlert`, called by whoever ran `checkSensors()`

**Status:** Accepted · **Date:** 2026-10-13

## Context
The team drew `+ addAlert(a: Alert): void` on `User` (commit `6b8ec97`). That closes gap G1 of the
[v3 analysis](../../../research/2026-10-13-cabin-monitor-input-set-analysis-v3.md): there is now a
way for an alert to reach a user.

The drawings do not say who calls `addAlert`:
- `Cabin` has no relation to `User`; `User *-- Cabin` is navigable from the user only;
- in `sequence-check_sensors.drawio`, `Cabin` returns `alerts` to the actor and makes no call to
  `User`.

## Decision
- `Cabin::checkSensors()` returns the alerts and never touches a `User`.
- The **caller** of `checkSensors()` delivers each returned alert with `User::addAlert(a)`. That
  caller is the application loop; `demo.cpp` shows it.
- `scenario()` in `scenarios/check_sensors.cpp` does **not** call `addAlert`. The diagram does not
  draw that call, and making it would add a message.
- ADR-0005 still holds: `AlertSent` fires inside `checkSensors()`, at the moment the alerts are
  returned.

## Consequences
- No undrawn association from `Cabin` to `User`, and no extra sequence message.
- An application that forgets to call `addAlert` loses the alerts. This is documented on
  `Cabin::checkSensors()` and `User::addAlert()`.
- If the team wants the cabin to deliver alerts itself, they need to draw `Cabin --> User` and the
  `addAlert` message, and this ADR would be superseded.
