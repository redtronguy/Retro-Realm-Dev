---
id: T-011
title: Concept mockups for the 15 main locations
owner: codex
status: todo
branch: codex/location-mockups
pr:
depends_on: []
---

## Goal
One SNES-style concept image per main location from *The Tidebound Count*, to set the game's art direction. These are **concept/mood references only** (decisions.md D7), not production art.

## Style rules (every image)
- Looks like a real 1990s Super Nintendo screenshot: native **256×224** upscaled 4× with nearest-neighbour (1024×896), crisp square pixels, no blur, no anti-aliasing, no modern lighting or bloom.
- Built from 16×16 tiles; each tile/sprite ≤ 15 colours + transparency; ~120 colours on screen; dithering for gradients.
- Camera: **top-down 3/4** like *Zelda: A Link to the Past* (Q1 default) for overworld/interiors. Set-piece locations (the Lock temple, the Sunspire summit) may use a dramatic side or angled view; label those "set piece".
- Quality bar: *Chrono Trigger*, *A Link to the Past*, *Secret of Mana*, *ActRaiser*. It must read as pixel art from 1995, not "pixel-art inspired" modern illustration. The user rejected an earlier attempt that looked too modern. When in doubt, use fewer colours and chunkier pixels.
- **No HUD** (keeps focus on the location). Include the hero (Edric, 16×24 sprite: dark hair, sailor's coat early, black coat in Solace) in each scene for scale, unless noted.
- Source of truth: `book/bible.md` and the canon chapters listed. Read them for details.

## Locations (file name → brief → where in the book)
1. `01-saltmere.png`: Harbour town at morning. White houses, red roofs climbing a hill; stone quays, a curved harbour wall with a light tower; the *Gull's Promise* (three-masted merchantman) docking; fishwives, gulls, crowds. (ch. 1–2)
2. `02-the-weir.png`: Marsh quarter at night. Stilt-houses of driftwood and reed joined by swaying plank walkways; hanging lamps; black channels; a small boat garlanded with sea-lavender and white shells at a landing stage. (ch. 3, 5, 31)
3. `03-gulls-promise.png`: Deck of the merchant ship at sea, sunset; captain's wheel, rigging, crew. (ch. 1)
4. `04-drowned-keep.png`: Exterior: black rock "the Teeth" rising from a grey sea; fused black outer walls; a tower with a red lantern; storm. (ch. 7, 12)
5. `05-keep-cells.png`: Interior: a damp stone cell 8×5 paces, straw pallet, slit window high up with a thin grey light, a stone block pulled out of the wall revealing a tunnel; an old man with a wild white beard (Faro) emerging. (ch. 7–10) Set piece allowed.
6. `06-the-lock.png`: Set piece: vast sea-cave temple; tiered ledges with ancient carvings of robed figures with raised hands; black water; seven enormous chains of solid gold light plunging from the roof into the water; a sea-gate barred with gold light. (ch. 11, 29–30)
7. `07-corrow.png`: Lawless smugglers' port: tarred shacks and taverns built into cliffs, crowded quay, the black two-masted cutter *Slip* with patched grey and brown sails. (ch. 14)
8. `08-cristal-ruins.png`: Isle of Cristal on discovery: hook-shaped harbour, fallen tower, empty terraces of pale glittering stone climbing to a ruined citadel with a broken dome; seabirds; overgrowth. (ch. 15)
9. `09-cristal-city.png`: Cristal rebuilt five years later: busy quays, market awnings, terraces with life, Weir-folk stilt-houses at the harbour's inner end, shore walls with watchtowers, the ugly little whitewashed temple with a gold sun on the third terrace; dusk, lamps lit. (ch. 18)
10. `10-dry-well-inn.png`: A mean, mud-coloured roadside inn on a flat marsh road in the rain; interior variant welcome (taproom, fire, a fat innkeeper, a grey-robed priest). (ch. 16)
11. `11-greyhills-monastery.png`: Bandit camp in a ruined monastery in a rocky ravine: broken cloister, caves, campfires, bandits, Sardhi grey horses arriving. (ch. 19, 26)
12. `12-grey-point.png`: Headland mirror tower: tall white tower with a great brass-mounted mirror flashing sunlight; a keeper's cottage, chickens, a little girl. (ch. 21)
13. `13-solace.png`: The white capital on cliffs: tiers of white walls and gold roofs, bridges over ravines, gardens; the impossibly tall slender **Sunspire** with a blazing gold point of light at its summit; mirror towers flashing. (ch. 19–20)
14. `14-house-blue-door.png`: The Count's house on the Street of Lanterns: courtyard with a black marble pool and a dancing fountain; hall with walls of green glass; lantern over a blue gate. (ch. 20)
15. `15-sunspire-summit.png`: Set piece: great round room of glass at the top of the tower, gold dome, a glowing well in the floor with seven gold chains rising out of it, a golden young king in white in a white chair holding a white cup. (ch. 27)

Stretch (if time): Larkmere's walled garden with the great chestnut tree at night (ch. 22); the Pale Cliffs dueling ground at dawn (ch. 23); the Valtor stillroom (ch. 22).

## Deliverables
- Images in `design/mockups/locations/` with the file names above.
- `design/mockups/locations/README.md`: a table of image, location, prompt used, and notes (what to keep or change).
- Open a PR into `main`; send a `review` message to claude and the user.

## Acceptance
- [ ] 15 images (stretch optional), consistent palette and camera across the set
- [ ] They pass the "would this fool someone as a real SNES screenshot?" test; reject and regenerate anything with smooth gradients, blur or non-grid pixels
- [ ] README with prompts, so images can be regenerated or iterated

## Notes
- 2026-10-12 claude: created at the user's request; assigned to codex. Claude's branch-chapter work (T-010) is in `book/branches/`. No overlap.
