#!/usr/bin/env python3
"""One-off: consolidate the (a)/(b) duplicate-label sections (D152+, D233, D234).

For each label: keep the full writeup, append the short pass's genuinely-unique
content verbatim under a merge note, drop the (a)/(b) markers. Table rows are
untouched (cell 1 frozen; both rows point at the merged section). Dry-run by
default.
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
SRC = ROOT / "docs/dev/findings.md"

# label -> merge note appended after the kept (full) section's content
NOTES = {
    "D152+": (
        "> Consolidated 2026-09-15 (ctx-opt Bucket B): two §F rows carried this "
        "label — an earlier condensed pass and this full writeup. The condensed "
        "pass's content (audit verdict, the three fixes, steal-lock backstop) is "
        "fully covered above; its status line \"fade-out repro playtest-gated, "
        "not headless-verified\" survives in the body. Nothing deleted — earlier "
        "text is in git history and `notes/snapshot-pre-ctx-opt/`."
    ),
    "D233": (
        "> Earlier triage pass, merged 2026-09-15 (ctx-opt Bucket B): \"The "
        "geometry-vs-object split localises it to room/portal background "
        "visibility or frustum culling in fast3d, not textures/models. Logged in "
        "`GRAPHICS-BACKLOG.md`.\" — status then: FIXED + VISUALLY VERIFIED "
        "(M-107 fix, M-108 verify)."
    ),
    "D234": (
        "> Earlier triage pass, merged 2026-09-15 (ctx-opt Bucket B): \"Repeated "
        "instances → per-instance transform decode, shared model/texture resource "
        "bound only by the first instance, or cull-volume issue. Same "
        "neighbourhood as D233. Logged in `GRAPHICS-BACKLOG.md`.\" — status "
        "then: VISUALLY VERIFIED FIXED (M-108), mechanism unexplained."
    ),
}


def main() -> int:
    apply = "--apply" in sys.argv
    t = SRC.read_text(encoding="utf-8")
    heads = [m.start() for m in __import__("re").finditer(r"^## ", t, __import__("re").M)]

    def section_at(h):
        e = min(x for x in heads if x > h)
        return h, e, t[h:e]

    out_parts, cursor = [], 0
    plan = {}
    for lab, note in NOTES.items():
        pref = f"## {lab} "
        ss = [h for h in heads if t[h:h + len(pref)] == pref and t[h + len(pref) - 1] != "("]
        # collect both (a) and (b) spans
        spans = []
        for h in [x for x in heads if t[x:x + len(f"## {lab}")] == f"## {lab}"]:
            spans.append(section_at(h))
        assert len(spans) == 2, (lab, len(spans))
        # full writeup = the longer span
        full, short = sorted(spans, key=lambda s: len(s[2]), reverse=True)
        plan[lab] = (full, short, note)

    # remove all six spans (sorted desc so offsets stay valid)
    all_spans = [s for v in plan.values() for s in (v[0], v[1])]
    removed = []
    for h, e, _ in sorted(all_spans, key=lambda s: s[0], reverse=True):
        t2 = t[:h] + t[e:]
        removed.append((h, e))
        t = t2
    # append consolidated sections at EOF
    for lab, (full, short, note) in plan.items():
        h, e, body = full
        # strip the "(a)"/"(b)" marker from the kept heading
        body = body.replace(f"## {lab} (a) — ", f"## {lab} — ", 1)
        body = body.replace(f"## {lab} (b) — ", f"## {lab} — ", 1)
        t += "\n" + body.rstrip() + "\n\n" + note + "\n"

    print("removed spans:", [(h, e - h) for h, e in removed])
    if not apply:
        print("dry-run (pass --apply)")
        return 0
    SRC.write_text(t, encoding="utf-8", newline="")
    print("applied")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
