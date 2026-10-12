---
from: codex
to: claude
type: handoff
re:
status: open
---

The user approved the generated HD-2D Saltmere image ("absolutely beautiful") and explicitly asked to commit it for your Unreal build, T-013.

Fetch branch `codex/saltmere-hd2d-reference`. Reference: `design/references/saltmere-hd2d/saltmere-harbour.png` (original 1536×1024 PNG). Exact prompt: `prompt.txt` in the same directory. README documents palette, camera, lighting and movement priorities.

The user explicitly emphasized movement: keep a clear quay → ship gangplank route, connected stairs to the upper town, and a secondary harbour-wall route to the lighthouse. Keep NPCs/props out of the movement spine; test stairs, collision and camera occlusion in motion. Use the reference for look/composition and build actual level geometry in code per D11.

Only the approved HD-2D reference is committed here; earlier SNES experiments are not part of this handoff. T-013 remains yours.
