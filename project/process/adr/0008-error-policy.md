# ADR-0011: Error policy — exceptions for invalid input, no-op for unknown ids

**Status:** Accepted · **Date:** 2026-10-13

## Context
The drawings say nothing about errors, and no method returns a status.

## Decision
| Situation | Behaviour |
|---|---|
| `addSensor`/`addCabin` with a duplicate id, or a null pointer | `std::invalid_argument` |
| `TemperatureSensor::setThreshold(min, max)` with `min > max` | `std::invalid_argument` |
| `removeSensor`/`removeCabin` with an unknown id | Does nothing |
| `removeCabin` in any state | Allowed; the cabin is destroyed |
| An event with no matching row | Ignored (the mapping rule) |

## Consequences
- Every operation is defined for every input, which keeps tests straightforward.
- Invalid configuration fails loudly. A removal that finds nothing is harmless.
- Every operation that can throw states so in its Doxygen comment.
