# ADR-0003: Ownership — `unique_ptr` for sensors and cabins, by-value readings and alerts

**Status:** Accepted · **Date:** 2026-10-13

## Context
Every relation in the class diagram is a composition: `Cabin *-- "*" Sensor`,
`User *-- "*" Cabin`, `Sensor *-- "*" Reading`, `User *-- "*" Alert`. The mapping
([UML-CPP-MAPPING.md](../../../tools/umlverify/docs/UML-CPP-MAPPING.md#relations)) accepts a
`unique_ptr` or a by-value member. `Sensor` is abstract, so it cannot be held by value.

## Decision
| Member | Type |
|---|---|
| `Cabin::sensors_` | `std::vector<std::unique_ptr<Sensor>>` (polymorphic) |
| `User::cabins_` | `std::vector<std::unique_ptr<Cabin>>` (stable addresses for `getCabins()`) |
| `Sensor::readings_` | `std::vector<Reading>`, **protected** so that subclasses' `read()` can append |
| `User::alerts_` | `std::vector<Alert>` |

`addSensor` and `addCabin` take a `std::unique_ptr`, which transfers ownership.
`getSensors()` and `getCabins()` return `std::vector<T*>`. A `vector<unique_ptr<T>>` return
type would normalize to `list<unique_ptr<T>>` and not match `T[]`.

## Consequences
- No `shared_ptr` anywhere, so ownership and lifetime are clear.
- Pointers from `getSensors()`/`getCabins()` become dangling after `remove*`. This is
  documented in the headers.
- `readings_` being protected costs nothing in the report, because relations carry no visibility.
