---
id: T-003
title: Wire protocol crate and authoritative server
owner: unassigned
status: todo
branch:
pr:
depends_on: [T-001, T-002]
---

## Goal
`realm-proto` (framed binary messages, no_std) and `apps/server` (tokio TCP or WebSocket, fixed 10 Hz tick, snapshots and tile deltas, save/load). Plus a Dockerfile and a multi-arch image built in CI.

## Acceptance
- [ ] In-memory client<->server integration test
- [ ] Docker image builds for amd64 + arm64 in CI

## Notes
- 2026-10-09 claude: created. Proposed owner: codex. The user confirms the split (decisions.md Q7).
