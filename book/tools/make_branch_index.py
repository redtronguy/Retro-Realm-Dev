"""Generate book/branches/README.md (the gamebook index) from the branch files."""
import glob, os, re

ACTS = [
    ("act-1", "Act I: The Betrayal"),
    ("act-2", "Act II: The Keep"),
    ("act-3", "Act III: The Count of Cristal"),
    ("act-4", "Act IV: The Three Hands"),
    ("act-5", "Act V: The Sun King"),
]
root = os.path.join(os.path.dirname(__file__), "..", "branches")
out = ["# The Tidebound Count: Branches", "",
       "What happens if Edric chooses differently. Each event below branches from the canon novel at a decision point; "
       "each file names the exact moment, links back to the canon path, and plays out every other option, "
       "with a **Later** section showing how the choice comes back.", "",
       "Each option is marked with its effect on the two dials from the game design:",
       "- **Heart**: + is mercy, − is vengeance.",
       "- **Tide**: + is taking or keeping the sea god's power, − is giving it back.", "",
       "Branches are independent \"what ifs\": each one assumes canon everywhere except its own choice.", "",
       "Endings: the canon ending is [Chapter 31](../31-wait-and-hope.md). Codex's *The Sea That Keeps* is in "
       "[alternate-endings](../alternate-endings/README.md). The rest are in Act V below.", ""]
for d, title in ACTS:
    files = sorted(glob.glob(os.path.join(root, d, "*.md")))
    if not files:
        continue
    out += [f"## {title}", ""]
    for f in files:
        text = open(f, encoding="utf-8").read()
        h1 = re.search(r"^# (.+)$", text, re.M)
        name = h1.group(1) if h1 else os.path.basename(f)
        rel = f"{d}/{os.path.basename(f)}"
        opts = []
        for m in re.finditer(r"^## (.+)\n+(\*Heart[^\n]*\*)?", text, re.M):
            if m.group(2):
                opts.append(f"{m.group(1)} ({m.group(2).strip('*')})")
        out.append(f"- **[{name}]({rel})**")
        for o in opts:
            out.append(f"  - {o}")
    out.append("")
open(os.path.join(root, "README.md"), "w", encoding="utf-8").write("\n".join(out))
print("index written")
