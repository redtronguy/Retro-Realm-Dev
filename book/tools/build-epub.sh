#!/usr/bin/env bash
# Builds book/dist/The-Tidebound-Count.epub from the chapter files.
# Requires pandoc (3.x) and python3 with Pillow.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p dist
python3 tools/make_cover.py dist/cover.jpg
cat > dist/metadata.yaml <<META
---
title: The Tidebound Count
subtitle: A Novel
creator:
  - role: author
    text: Retro Realm
language: en-US
rights: All rights reserved.
description: A sailor betrayed on his wedding night is buried alive in the Drowned Keep. Fourteen years later, the sea god chained beneath the prison sets him free with a price.
---
META
pandoc dist/metadata.yaml [0-3][0-9]-*.md \
  --from markdown --to epub3 \
  --epub-cover-image=dist/cover.jpg \
  --css tools/epub.css \
  --toc --toc-depth=2 --split-level=2 \
  -o dist/The-Tidebound-Count.epub
echo "built: book/dist/The-Tidebound-Count.epub"

# --- Branches EPUB ---------------------------------------------------------
if [ -d branches ]; then
  python3 tools/make_branch_index.py
  python3 tools/make_cover.py dist/cover-branches.jpg "BRANCHES"
  tmp=dist/branches-src; rm -rf "$tmp"; mkdir -p "$tmp"
  cat > "$tmp/00-intro.md" <<'INTRO'
# How to read this book

Every event here branches from the novel *The Tidebound Count* at a moment where Edric could have chosen differently. Each event says where it splits from the canon story, then plays out the other choices, with a **Later** section showing how each choice comes back.

Each choice shows its effect on two dials: **Heart** (+ mercy, − vengeance) and **Tide** (+ taking the sea god's power, − giving it back).

Each branch is its own "what if", assuming the canon story everywhere except its own choice.
INTRO
  n=1
  for act in act-1 act-2 act-3 act-4 act-5; do
    [ -d "branches/$act" ] || continue
    case $act in
      act-1) t="Act I: The Betrayal";; act-2) t="Act II: The Keep";;
      act-3) t="Act III: The Count of Cristal";; act-4) t="Act IV: The Three Hands";;
      act-5) t="Act V: The Sun King";;
    esac
    printf '# %s\n' "$t" > "$tmp/$(printf %02d $n)-0-$act.md"
    for f in branches/$act/*.md; do
      # demote headings one level so events nest under their act
      sed -e 's/^### /#### /' -e 's/^## /### /' -e 's/^# /## /' "$f" > "$tmp/$(printf %02d $n)-$(basename "$f")"
    done
    n=$((n+1))
  done
  cat > "$tmp/metadata.yaml" <<META
---
title: "The Tidebound Count: Branches"
subtitle: What happens if Edric chooses differently
language: en-US
---
META
  pandoc "$tmp/metadata.yaml" "$tmp"/[0-9]*.md --from markdown --to epub3 \
    --epub-cover-image=dist/cover-branches.jpg --css tools/epub.css \
    --toc --toc-depth=2 --split-level=2 -o dist/The-Tidebound-Count-Branches.epub
  rm -rf "$tmp"
  echo "built: book/dist/The-Tidebound-Count-Branches.epub"
fi
