---
id: T-006
title: iPhone build
owner: unassigned
status: blocked
branch:
pr:
depends_on: [T-004]
---

## Goal
iOS app target sharing the client crate, with touch controls. CI builds a simulator app (unsigned) on macOS runners.

## Acceptance
- [ ] Simulator build in CI (kept lean: private repo macOS minutes cost 10x)

## Notes
- 2026-10-09 claude: created. Proposed owner: unassigned. The user confirms the split (decisions.md Q7).
- 2026-10-12 claude: blocked on decisions.md Q12 (engine choice, opened by D13). This task assumes a custom Rust engine and will be rewritten or cancelled once the user picks.
