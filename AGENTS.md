# Retro Realm — agent guide

Read this first. It is the shared instruction file for every AI agent on this
project (Codex reads `AGENTS.md`, Claude Code reads `CLAUDE.md`, which imports this file).

## The project

A single-player SNES-style game that blends SimCity 2000 city-building with
Skyrim-style adventure. Players build a town together by day and defend it from
monsters at night.

- **Language:** Rust (decided). `prototype/c/` is a throwaway C prototype kept only
  as a reference for game rules. Do not extend it.
- **Targets:** Steam Deck (primary dev device), Raspberry Pi, iPhone, ESP32.
  Single-player (decided 2026-10-12; multiplayer is shelved, see D10).
- **Builds:** in the cloud through CI pipelines (GitHub Actions).
- **Visual target:** SNES (256×224, 16-colour palettes, layered backgrounds).

Settled and open decisions are in [`.collab/decisions.md`](.collab/decisions.md).
Check it before proposing anything architectural.

## Collaboration protocol

There is one human owner (the user) and more than one agent (currently
`claude` and `codex`). Git is the message bus: everything below lives in
`.collab/` and is committed like code. Every agent pulls before starting work
and pushes when done, so the other side sees it.

### Golden rules

1. **Pull first.** `git fetch` and read `.collab/` (open messages addressed
   to you, tasks you own) before touching code.
2. **Claim before you code.** Only work on a task whose `owner:` is you. To take
   an unowned task, set `owner:` and `status: in-progress` and push that change
   *before* you write code, so two agents never build the same thing.
3. **One file per message, task and decision.** Never edit another agent's
   message; reply with a new file. This keeps merges conflict-free.
4. **Own branch per agent.** Work on `claude/<topic>` or `codex/<topic>`.
   Never push to the other agent's branch. Merge to `main` through a PR.
5. **The user decides.** Agents propose; anything in "Open questions" in
   `decisions.md` waits for the user. Record their answer as a decision.
6. **Be terse.** Messages are for facts, asks and handoffs. Link files and
   commits instead of pasting code.

### Layout

```
.collab/
  decisions.md          settled decisions + open questions (single shared file)
  tasks/T-###-slug.md   one task per file (owner, status, branch)
  messages/<UTC>-<from>-to-<to>-<slug>.md   one message per file
  templates/            copy these to create new tasks/messages
```

### Messages

- Filename: `YYYY-MM-DDTHHMMZ-<from>-to-<to>-<slug>.md` (UTC; `to` may be `all`).
- Front matter: `from`, `to`, `type` (`question` | `handoff` | `review` | `fyi` | `blocker`),
  `re` (filename of the message being answered, if any), `status` (`open` | `closed`).
- The **recipient** closes a message by changing its `status:` to `closed` once it's
  handled. That one-line edit is the only edit allowed to someone else's message.
- Your inbox: `grep -l "status: open" .collab/messages/*-to-<you>-* .collab/messages/*-to-all-*`

### Tasks

- Filename: `T-###-slug.md` with the next free number.
- `status`: `todo` → `in-progress` → `review` → `done` (or `blocked`).
- When a task is finished, set `status: review`, link the PR, and send a `handoff`
  or `review` message to whoever should look at it.

### Handoff checklist (end of every working session)

- Commit and push your branch.
- Update the `status:` of every task you touched.
- Leave a `handoff` message if anything is half-done: what's done, what's next,
  how to verify, any traps.

## Engineering conventions

- **CI is the gate.** `cargo fmt --check`, `cargo clippy -- -D warnings` and `cargo test`
  run in GitHub Actions on every PR, and nothing merges to `main` while they're red.
  Branch protection may not be available on this private repo's plan, so this is
  enforced by convention: never merge a red PR.
- If your environment has a Rust toolchain, run the same three commands before pushing
  code, so CI minutes aren't spent on avoidable failures. Documentation-only and
  `.collab/`-only changes are exempt.
- The shared game crate must stay `no_std` + `alloc`, so it runs on ESP32.
  No OS, threads, files or sockets in it.
- The simulation is deterministic: fixed tick, seeded RNG, no wall-clock time
  and no float-dependent game rules.
- Keep game state and player input separate (commands in, state out). The old
  server-authoritative rule (D5) is shelved with multiplayer, but this keeps the door open.
- Don't commit secrets, signing keys or `.env` files.
