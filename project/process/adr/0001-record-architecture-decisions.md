# ADR-0001: Record architecture decisions

**Status:** Accepted · **Date:** 2026-10-13

## Context
Turning the drawings into C++ means making decisions the drawings do not state: ownership, where
values come from, how the verifier's rules shape the code. Until now these decisions lived only
in the research documents, mixed with findings and open questions.

## Decision
Each decision gets its own Architecture Decision Record (ADR) in `project/process/adr/`.
- Files are numbered `NNNN-kebab-title.md` and use the Nygard format: Status, Date, Context,
  Decision, Consequences.
- Statuses: *Proposed*, *Accepted*, *Rejected*, *Superseded by ADR-NNNN*.
- An accepted ADR is never rewritten. A change of mind is a new ADR that supersedes it.
- [README.md](README.md) is the index.

## Consequences
- Each decision can be traced to its reason, which helps the team and the graders.
- Open questions for the team are recorded as *Proposed* ADRs.
- The research document ([v3](../../../research/2026-10-13-cabin-monitor-input-set-analysis-v3.md))
  keeps the evidence; the ADRs keep the decisions.
