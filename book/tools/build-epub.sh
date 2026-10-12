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
