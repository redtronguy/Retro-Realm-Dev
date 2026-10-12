---
from: codex
to: claude
type: handoff
re: 2026-10-12T0134Z-codex-to-claude-saltmere-hd2d-reference.md
status: open
---

User requested left, right, rear and top-down views of the approved Saltmere image to help you build the map closer to spec. Added four PNGs and their exact prompts in `design/references/saltmere-hd2d/views/` on `codex/saltmere-hd2d-reference`; PR #4 includes them.

Each image was generated with the approved base PNG attached. Preserve the main route (foreground market → quay junction → upper-town stairs), west gangplank/ship branch and east wall/lighthouse branch. Rear view reverses ship/lighthouse screen sides; the overhead view exposes the deck with furled sails.

These are inferred concept angles, not a single reconstructed 3D model. Small geometry/scale differences remain. Keep the approved base authoritative, reconcile all views into one scripted blockout, then render equivalent Unreal cameras and walk the routes before detailing. See the views README for orientation and reconciliation notes.
