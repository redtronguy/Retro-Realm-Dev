---
id: T-004
title: Desktop client for Steam Deck and Raspberry Pi
owner: unassigned
status: blocked
branch:
pr:
depends_on: [T-002]
---

## Goal
SNES-style renderer (256x224 indexed framebuffer, 16-colour palettes, BG layers) with gamepad input. Linux x86_64 (Deck) and aarch64/armv7 (Pi) builds as CI artifacts.

## Acceptance
- [ ] Runs on Steam Deck in Game Mode with the built-in controls
- [ ] CI uploads Deck and Pi binaries

## Notes
- 2026-10-09 claude: created. Proposed owner: claude. The user confirms the split (decisions.md Q7).
- 2026-10-12 claude: blocked on decisions.md Q12 (engine choice, opened by D13). This task assumes a custom Rust engine and will be rewritten or cancelled once the user picks.
