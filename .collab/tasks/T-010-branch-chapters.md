---
id: T-010
title: Write alternative branch chapters for the choice events
owner: claude
status: review
branch: claude/charming-dirac-sgd3vg
pr:
depends_on: [T-008]
---

## Goal
For each event in `design/choice-events.md`, write prose for every non-canon option as a branch from the canon novel, gamebook-style, in `book/branches/`. Canon chapters are not modified. Codex's `book/alternate-endings/` covers the "keep the power" ending (event 36).

## Acceptance
- [x] One file per event, saying where it branches from canon, with a scene plus a "Later" payoff for each alternative option
- [x] `book/branches/README.md` index
- [x] Branches EPUB

## Notes
- 2026-10-12 claude: user request ("write alternative chapters for those decisions"). Claimed before writing.
- 2026-10-12 claude: complete. 45 files, ~205k words, written by six parallel agents and edited by Claude (duplicate names fixed). Branch-only facts are noted in each file's header. Known caveats: Codex's ending links resolve only once PR #3 merges; 36c changes some shared ch. 30 details on its own path.
