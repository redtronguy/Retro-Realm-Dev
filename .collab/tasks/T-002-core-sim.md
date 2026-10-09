---
id: T-002
title: Port game simulation to no_std realm-core
owner: unassigned
status: todo
branch:
pr:
depends_on: [T-001]
---

## Goal
Port world generation, city growth, monsters, combat and building rules from `prototype/c/src/rr_world.c` into `realm-core` (no_std + alloc, deterministic).

## Acceptance
- [ ] Same seed gives the same map and tick results (tests)
- [ ] Unit tests: zone growth, build costs, raids, combat, respawn

## Notes
- 2026-10-09 claude: created. Proposed owner: claude. The user confirms the split (decisions.md Q7).
