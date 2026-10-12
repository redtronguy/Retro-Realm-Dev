# Decisions

Settled decisions are final unless the user reopens them. Agents may add to
**Open questions**; only the user's answer moves an item into **Decided**.

## Decided

| # | Date | Decision | Why / notes |
|---|------|----------|-------------|
| D1 | 2026-10-09 | Implementation language is **Rust**. | User preference. The C prototype in `prototype/c/` is reference only. |
| D2 | 2026-10-09 | Visual target is **SNES**, not NES. | 256×224, 15-bit colour, 16-colour palettes (8 BG + 8 sprite), up to 4 BG layers, sprites up to 64×64, colour math/transparency. |
| D3 | 2026-10-09 | Platforms: **Steam Deck** (primary dev), **Raspberry Pi**, **iPhone**, **ESP32**. ~~Plus a separate multiplayer server.~~ | Server dropped by D10. |
| D4 | 2026-10-09 | All builds run in the cloud through **CI pipelines** (GitHub Actions). | Private repo: macOS (iOS) minutes are billed at 10×, so keep iOS jobs lean. |
| D5 | 2026-10-09 | ~~Server is authoritative; clients send commands and receive snapshots and deltas.~~ | **Shelved by D10.** Keep the command/state separation in the core so multiplayer could return later. |
| D6 | 2026-10-09 | ~~Core gameplay loop: build the city (roads, R/C/I zones growing by demand, shared treasury) by day; adventure and defend at night (monsters raid buildings into rubble; loot feeds the treasury).~~ | **Superseded by D12.** |
| D7 | 2026-10-09 | Art is authored in **Aseprite**, imported by a build step. AI images are concept/mood references only. | |
| D8 | 2026-10-11 | Story basis: ***The Tidebound Count***, an original fantasy reimagining of *The Count of Monte Cristo* (public domain) with powers. Claude writes the novel first in `book/`; the game is adapted from it after the user has read it. | User request. Bible: `book/bible.md`. Don't adapt game content from the book until the user signs off. |
| D9 | 2026-10-12 | **Choices are a core mechanic**, like *Fable*: moral choices shape the hero, a skill tree has good and bad paths, and players can pursue different paths and endings. | User request, inspired by Codex's alternate ending. Proposal: `design/choices-and-morality.md` (two dials, Tide and Heart; four endings; Warden vs Tide-taken skill branches). Details are open questions Q8–Q11. |
| D10 | 2026-10-12 | **Single-player for now. Multiplayer is shelved.** No server, no networking, no co-op. | User decision. T-003 (protocol/server) is cancelled. The core stays deterministic, with command/state separation, so multiplayer could come back later. |
| D11 | 2026-10-12 | **Levels are designed in code**, not from drawn or AI-generated mockups. | User decision. T-011 (location mockups) is cancelled. The 15 location briefs in that task remain a useful reference for what each level must contain. |
| D12 | 2026-10-12 | **No city-builder.** The game is a pixel-art blend of *Skyrim* and *Fable* (open-world action RPG with moral choices), based on the book *The Tidebound Count*. | User decision. Supersedes D6. The city-builder ties in `design/choices-and-morality.md` and `design/choice-events.md` need reworking (T-012). The C prototype's city rules are no longer a reference. |
| D13 | 2026-10-12 | **A compelling game comes before the hardware it runs on.** Hardware targets no longer constrain the engine or architecture; an off-the-shelf engine is allowed. | User decision. Lifts the `no_std`/ESP32 constraint on the design. Which engine, and so which of the D3 platforms survive, is Q12. The user has an Unreal server available and is open to setting up an environment for something else. |

## Open questions (waiting on the user)

- **Q1 Camera:** top-down 3/4 (Zelda: ALttP) or isometric (SimCity 2000)? Default if unanswered: top-down. The user is comparing ChatGPT concept images. (After D12 there is no city-builder, so the SimCity argument for isometric is gone.)
- ~~**Q2 Roles:**~~ Closed by D10: the one player both builds and fights.
- ~~**Q3 World model:**~~ Closed by D10 (single-player).
- **Q4 Combat:** bump-to-attack (prototype) or action combat? Separate dungeon interiors?
- **Q5 Progression:** quests, factions, crafting, shop economy? (Partly answered by D9: a morality-driven skill tree.)
- **Q6 ESP32 hardware:** which board and display? ESP32-S3 with PSRAM is the comfortable SNES target.
- **Q7 Work split** between Claude and Codex (see `tasks/`).
- **Q8 Morality model:** two dials (Tide + Heart, four endings) or one good/evil line? See `design/choices-and-morality.md`.
- **Q9 Player character:** play as Edric, or as a new Tidebound hero in Edric's world?
- **Q10 Reversibility:** fully reversible with effort, or locked after key choices?
- ~~**Q11 Multiplayer choices:**~~ Closed by D10 (single-player).
- **Q12 Engine:** which engine, now that D13 allows one? Claude recommends **Godot 4 with GDScript**: first-class 2D pixel art, tilemaps, UI and dialogue; scenes and scripts are text files agents can write and review; exports to Steam Deck, iPhone and Raspberry Pi from headless CI. Alternatives: **Bevy** (keeps Rust, D1, but no editor and slower content iteration) or **Unreal** (user has a server; weak 2D, binary assets agents can't edit, no Pi; only makes sense for a 3D/HD-2D look). Choosing Godot or Unreal reopens D1 (Rust) and drops ESP32 from D3. T-001, T-004, T-005, T-006 and T-007 are blocked on this.
