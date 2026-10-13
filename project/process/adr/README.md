# Architecture Decision Records

The decisions behind turning the drawings in [`project/diagrams/input/`](../../diagrams/input/)
into C++. The evidence for each one is in
[research/2026-10-13-cabin-monitor-input-set-analysis-v3.md](../../../research/2026-10-13-cabin-monitor-input-set-analysis-v3.md).
The format is set by [ADR-0001](0001-record-architecture-decisions.md).

| ADR | Decision | Status |
|---|---|---|
| [0001](0001-record-architecture-decisions.md) | Record architecture decisions | Accepted |
| [0002](0002-drawings-are-ground-truth.md) | The drawings are ground truth; implement exactly what is drawn | Accepted |
| [0003](0003-ownership-mapping.md) | Ownership: `unique_ptr` for sensors and cabins, by-value readings and alerts | Accepted |
| [0004](0004-table-driven-state-machine.md) | Table-driven state machine inside `Cabin` | Accepted |
| [0005](0005-checksensors-fires-events.md) | `checkSensors()` fires `MeasurementIntervalElapsed` and `AlertSent` | Accepted |
| [0006](0006-hardware-seam.md) | Sensor values and time come from a free-function hardware seam | Accepted |
| [0007](0007-friend-exception.md) | `friend` exception to keep the sequence trace exact | Superseded by 0011 |
| [0008](0008-error-policy.md) | Error policy: exceptions for invalid input, no-op for unknown ids | Accepted |
| [0009](0009-bounded-reading-history.md) | Bounded reading history (1 000 per sensor) | Accepted |
| [0010](0010-alert-delivery-by-caller.md) | Alerts reach a `User` through `addAlert`, called by whoever ran `checkSensors()` | Accepted |
| [0011](0011-friend-exception-revised.md) | Friend exception, revised to cover the sensor subclasses | Accepted |