---
id: T-005
title: Aseprite art import pipeline
owner: unassigned
status: blocked
branch:
pr:
depends_on: [T-001]
---

## Goal
Build step that converts .aseprite / PNG sprite sheets into indexed tiles and palettes for realm-core's renderer. Includes placeholder art.

## Acceptance
- [ ] Changing an .aseprite file updates the game without code edits
- [ ] Rejects art that breaks the SNES palette limits

## Notes
- 2026-10-09 claude: created. Proposed owner: codex. The user confirms the split (decisions.md Q7).
- 2026-10-09 codex (proposal; owner remains unassigned): Coordinate an asset contract with T-004 before coding: tile/sprite dimensions, palette-bank IDs, transparency index, layer ordering, animation durations and stable asset IDs. Validate each assigned bank against 16 entries (15 visible colours plus transparency where applicable), 15-bit RGB representation and the decided 8 BG / 8 sprite bank budgets. Report the asset/frame/bank causing a failure; reject rather than silently quantize or reassign art.
- 2026-10-09 codex (proposal): Keep Aseprite authored sources and deterministic generated outputs separate. Pin the exporter/version and palette ordering in CI, preserve layers/tags/frame timing in an explicit manifest, and make runtime data usable without filesystem access. Confirm Aseprite CLI availability/licensing on Actions before relying on it; any PNG route should retain authored metadata and satisfy the .aseprite-change acceptance criterion. Add CI fixtures for over-budget art, transparency and repeatable output. AI concept images are not production assets.
- 2026-10-12 claude: blocked on decisions.md Q12 (engine choice, opened by D13). This task assumes a custom Rust engine and will be rewritten or cancelled once the user picks.
