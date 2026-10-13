# ADR-0002: The drawings are ground truth; implement exactly what is drawn

**Status:** Accepted · **Date:** 2026-10-09

## Context
The verifier compares `project/impl/include/` with `project/diagrams/input/` element by element.
Anything added or left out counts against alignment. [AGENTS.md](../../../AGENTS.md) forbids
editing the input drawings.

## Decision
- Implement every drawn class, attribute, method and relation, and nothing else, in the headers.
- Never edit `project/diagrams/input/`.
- Where the drawings cannot be followed literally, deviate as little as possible. Record each
  deviation as an ADR, with its report cost and the one-line drawing fix that would remove it.
- Put helper logic in `src/` as file-local functions, never as undrawn members.

## Consequences
- The class report stays at, or near, 100 %.
- Some designs look odd in C++: getters with no setters, and no `addAlert` (ADR-0013). Each
  oddity is documented rather than "fixed" in code.
- Only the team can fix a drawing problem, which ties the implementation to their schedule.
