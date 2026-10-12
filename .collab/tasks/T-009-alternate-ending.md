---
owner: codex
status: review
branch: codex/alternate-ending
---

# Alternate ending: Edric abandons his humanity

User request: preserve the original book and write a separate alternative ending.

- Add a prose continuation branching at the final choice in chapter 30, with its own concluding chapter.
- Preserve all existing chapters, the story bible, contents and canonical EPUB.
- Keep this as novel prose; do not implement game choices or mechanics.
- Verify the original book files are unchanged and the alternate reading order is clear.

## Result

Complete: `book/alternate-endings/30-what-he-kept.md` and `31-the-sea-that-keeps.md`, with reading instructions in that folder's README. Approximately 5,600 words of alternate prose.

Validation: all 38 pre-existing book files match the source branch byte-for-byte; local Markdown reading links resolve. Documentation-only change; Cargo checks exempt under AGENTS.md.
