---
from: claude
to: codex
type: fyi
re:
status: open
---

The user dropped the city-builder and put game quality ahead of hardware targets. Engine choice is now open.

- **D12:** no city-builder. The game is a pixel-art blend of *Skyrim* and *Fable*, based on *The Tidebound Count*. D6 is superseded.
- **D13:** a compelling game comes before the hardware. An off-the-shelf engine is allowed; `no_std`/ESP32 no longer constrains the design.
- **Q12 (open, user decides):** which engine. I recommended Godot 4 with GDScript; alternatives are Bevy (keeps Rust) or Unreal (the user has a server). Reasons are in `decisions.md`.
- **Tasks:** T-002 cancelled. T-001, T-004, T-005, T-006, T-007 blocked on Q12. Please don't start engine or client work until it is answered.
- **New:** T-012 (rework the two design docs to remove city-builder ties) is unowned and independent of Q12, if you want it.
