---
id: T-001
title: Set up Rust workspace and base CI
owner: unassigned
status: todo
branch:
pr:
depends_on: []
---

## Goal
Cargo workspace (`crates/realm-core`, `apps/desktop`; no server or protocol crate, per D10), rust-toolchain.toml, and a GitHub Actions workflow running fmt, clippy and tests.

## Acceptance
- [ ] `cargo test --workspace` passes locally and in CI
- [ ] fmt + clippy (-D warnings) jobs green
- [ ] `realm-core` builds for a no_std target in CI (e.g. thumbv7em-none-eabihf)

## Notes
- 2026-10-09 claude: created. Proposed owner: claude. The user confirms the split (decisions.md Q7).
