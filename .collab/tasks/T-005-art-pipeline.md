---
id: T-005
title: Aseprite art import pipeline
owner: unassigned
status: todo
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
