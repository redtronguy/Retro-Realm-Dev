---
id: T-003
title: Wire protocol crate and authoritative server
owner: unassigned
status: cancelled
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
- 2026-10-09 codex (proposal; owner remains unassigned): Keep realm-proto no_std and transport-independent; specify explicit little-endian fields, version negotiation, maximum frame/count limits, and exact payload consumption. Validate enum values, coordinates and IDs before applying a decoded message, and avoid partial state updates on malformed input. Add CI golden wire vectors and tests for fragmented/coalesced frames, truncation, oversized counts and rejected commands. The C framing is useful reference, not a compatibility commitment.
- 2026-10-09 codex (proposal): Give each connection bounded input/output queues and command rates; let one simulation owner validate commands and advance fixed 10 Hz ticks in a defined order. Keep transport selection open until platform requirements are checked. Test slow peers, disconnect cleanup and full-map recovery after delta overflow alongside the in-memory session acceptance check.
- 2026-10-09 codex (proposal): Coordinate the persistence contract with T-002 and the user's Q3 world-model decision. For resumable deterministic worlds, persist RNG and all state needed for continuation, version and validate saves, and replace files atomically. Test uninterrupted ticks versus save/load continuation. The prototype derives a new RNG state on load and calls city simulation (which may change tiles); do not carry those behaviors into Rust accidentally.
- 2026-10-12 claude: **cancelled.** The user shelved multiplayer (decisions.md D10). If it ever returns, Codex's notes above are the starting point.
