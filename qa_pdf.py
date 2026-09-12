# -*- coding: utf-8 -*-
"""Geometric + textual QA for the generated PDF."""

import sys

import pdfplumber
from PIL import Image


def overlaps(a, b, tol=1.0):
    ax0, atop, ax1, abot = a
    bx0, btop, bx1, bbot = b
    ix = min(ax1, bx1) - max(ax0, bx0)
    iy = min(abot, bbot) - max(atop, btop)
    return ix > tol and iy > tol


def main(path, png_prefix):
    issues = []
    with pdfplumber.open(path) as pdf:
        print("pages:", len(pdf.pages))
        for i, page in enumerate(pdf.pages, 1):
            W, H = page.width, page.height
            words = page.extract_words(use_text_flow=False, keep_blank_chars=False)
            print("\n--- page %d  (%d x %d)  words=%d" % (i, W, H, len(words)))

            # bounds check
            for w in words:
                if w["x0"] < 30 or w["x1"] > W - 30 or w["top"] < 20 or w["bottom"] > H - 20:
                    issues.append("p%d out-of-safe-area: %r bbox=(%.1f,%.1f,%.1f,%.1f)"
                                  % (i, w["text"], w["x0"], w["top"], w["x1"], w["bottom"]))

            # overlap check between words on different lines
            for j in range(len(words)):
                for k in range(j + 1, len(words)):
                    a, b = words[j], words[k]
                    ra = (a["x0"], a["top"], a["x1"], a["bottom"])
                    rb = (b["x0"], b["top"], b["x1"], b["bottom"])
                    if overlaps(ra, rb, tol=0.8):
                        issues.append("p%d overlap: %r <> %r" % (i, a["text"], b["text"]))

            # vertical extent of content
            if words:
                top = min(w["top"] for w in words)
                bot = max(w["bottom"] for w in words)
                print("content top=%.1f bottom=%.1f  (page H=%.1f)" % (top, bot, H))
                print("bottom whitespace=%.1f" % (H - bot))

            # sample text
            txt = page.extract_text() or ""
            preview = txt.replace("\n", " | ")[:160]
            print("text:", preview)

            im = Image.open("%s-%d.png" % (png_prefix, i)).convert("L")
            px = im.load()
            w, h = im.size
            nonwhite = sum(1 for y in range(0, h, 3) for x in range(0, w, 3) if px[x, y] < 245)
            total = len(range(0, h, 3)) * len(range(0, w, 3))
            print("ink coverage=%.1f%%" % (100.0 * nonwhite / total))

    print("\n=== ISSUES: %d ===" % len(issues))
    for s in issues[:60]:
        print(s)
    return 1 if issues else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2]))
