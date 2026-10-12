# Choices and morality: design proposal

**Status:** draft for the user's review. Nothing here is decided until it's recorded in `.collab/decisions.md`.

**Origin:** the user's idea (inspired by *Fable*'s good/evil paths and skill trees), sparked by Codex's alternate ending *The Sea That Keeps*. Based on *The Tidebound Count* (`book/`) and its alternate ending (`book/alternate-endings/`).

## The core idea: two dials, not one

Fable uses one good/evil line. The novel suggests that's not quite enough. It asks two separate questions, and the two endings answer them differently:

1. **What do you do with power?** Keep the Deep's strength, or give it back.
2. **What do you do to people?** Mercy or vengeance.

In Claude's ending, Edric gives everything back and chooses mercy. In Codex's ending, he chooses mercy but keeps the power, because he can't bear to stop protecting people, and becomes a loving tyrant. Kindness and corruption are different things, so the game tracks them separately.

| Dial | One end | Other end | Moved by |
|---|---|---|---|
| **The Tide** | *Given*: you hold little of the Deep | *Kept*: you hold a lot of it | Using powers, choosing to take or keep power, "Return" rituals |
| **The Heart** | *Mercy* | *Vengeance* | How you treat people in story choices, combat, and running the city |

Both dials are visible and both are reversible, but reversing gets harder the further you've gone. That matches the book: giving back hurts, and the more you've taken, the more it hurts.

## Four paths and four endings

| | **Mercy** | **Vengeance** |
|---|---|---|
| **Given** | **The Tidewarden** (the novel's ending). You free the Deep, give it all back, and sail west with a whisper of the sea. | **The Hollow Count.** You give the power back but keep the hatred. You take the Sunspire's empty throne as an ordinary man, win everything, and are alone. |
| **Kept** | **The Sea That Keeps** (Codex's ending). The world becomes safe, controlled and loved. Nobody is free, and you no longer understand why Mira cries. | **The Flood.** You let the wave come. Every coast is drowned, guilty and innocent alike, and you walk into the sea laughing, like the Warden of Ys. |

There's also a **secret fifth ending** for players who keep the Tide low and the Heart high and make one specific sacrifice. The bible already hints at this as "Faro's other way".

## How the dials show (cheap on SNES and ESP32)

- **Tidemark on your sprite:** blue lines spread across the character as the Tide rises: chest, then arms, then throat, then face, then eyes. Done with palette swaps and overlay tiles, so it costs almost nothing in memory.
- **Eyes:** grey at a low Tide; sea-black at a high Tide, like the face in the cracked mirror in chapter 25.
- **World palette:** a high Tide makes nights darker and stormier; a low Tide makes the sea clearer and brings fish back.
- **NPCs:** high Heart brings greetings, gifts and volunteers. Vengeance brings fear: people step aside, doors close and prices change.

## Skill tree: three disciplines, two branches each

Each discipline from the book (Riptide, Undertow, Still Water) splits into a **Warden** branch, which needs a low Tide, and a **Tide-taken** branch, which needs or raises a high Tide. Some skills also need a Heart threshold.

| Discipline | Warden branch (low Tide) | Tide-taken branch (high Tide) |
|---|---|---|
| **Riptide** (force) | *Lift*: raise ships off reefs, carry allies, shield walls (the siege of Cristal) | *Drown*: water blades, crushing waves, the Flood |
| **Undertow** (pull) | *Mend*: draw blight out of the sick (Grey Point), read trade currents to boost the city's economy | *Sink*: drain wells, sink ships, curse wealth (Dorran's ruin) |
| **Still Water** (reflection) | *Truthsight and Sleep*: see lies, grant the saving sleep (Lyse) | *Mask and Compel*: wear any face, quiet a person's will (the coercion in Codex's ending) |

Dark skills are deliberately stronger in raw power. Light skills open more of the world: more allies, more city growth and more quests. That's the trade-off Fable got right.

## Ties to the city-builder (decision D6)

- **The Heart affects the city.** Mercy draws immigrants (Weir-folk, refugees) and night-watch volunteers. Vengeance means lower unrest and higher taxes, but people leave and nobody volunteers.
- **The Tide affects the nights.** At a low Tide you can calm some blighted things (fewer raids, but fights take longer). At a high Tide you can wipe out whole raids, but the blight spreads faster.
- **Rubble from raids:** rebuilding fast with Riptide raises the Tide. Rebuilding by hand with citizens costs time and money and raises the Heart.

## The big choices (from the novel's beats)

Each of these is a branching scene. The book took one path; the game offers the others.

| Scene (chapter) | Choices | Consequence |
|---|---|---|
| The priest at the inn (ch. 16) | Give Cade the diamond / withhold it / reveal yourself | Cade's fate, the jeweller's murder, whether Cade later warns you |
| The house of Moren (ch. 17) | Save Moren in secret / openly / not at all | Maxim's loyalty, the Moren fleet as an ally |
| Mazrah (ch. 18) | Free Hayda and let her choose / use her as a weapon | Whether Hayda testifies or leaves, and Hayda as a companion |
| The stillroom (ch. 22) | Tell Heloise about blackwater / refuse | **Whether Pip lives.** This is the novel's moral pivot |
| Cade at the window (ch. 25) | Warn him about Andric / let him go out | Cade dies or survives, and your Heart moves |
| Dorran in the cave (ch. 26) | Forgive him / let him starve / take the money | The Blight Fund goes back to the towns or to you |
| The siege of Cristal (ch. 28) | Lift the fleet / drown it | Cassia Vell becomes an ally or an enemy, and the Radiants rebuild Cristal or not |
| The chains (ch. 30) | Give it back / keep it / let the wave come | The four endings above |

## Multiplayer (open question)

The world is shared but the dials are personal. Proposal:

- Each player has their own Tide and Heart, visible on their sprite.
- **The city's mood** is the average Heart of everyone who's played in it, so a vengeful player makes the whole town warier.
- **Story choices** are made by whoever is standing in the scene. For world-changing choices (the chains), one option is a vote. Another is giving the "Warden" role to one player per world.
- Co-op players are **the Tidebound**: people Edric pulled from the sea, each carrying a strand of the Mark (bible, co-op note).

## Questions for the user

1. **Two dials (Tide + Heart) or one Fable-style good/evil line?** Two dials give four endings; one gives two or three and is simpler.
2. **Is the player Edric, or a new Tidebound hero in Edric's world?** This affects how literally the novel's scenes are replayed.
3. **How reversible should it be?** Fully, with effort (Fable), or locked after key choices like the stillroom?
4. **Multiplayer story choices:** a vote, a "Warden" player, or each player's own copy of the story?
5. **Which endings to build first?** Suggestion: the Tidewarden and the Sea That Keeps, since both are already written as prose.
