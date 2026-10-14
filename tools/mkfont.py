#!/usr/bin/env python3
# Builds src/vnfont.c: a BKED-style 8x14 Vietnamese font.
#
# Glyphs traced from BKED (tools/bked_glyphs.txt) are used as-is. Vietnamese
# letters BKED's screenshots don't show are composed from the traced base
# letters with marks drawn the way BKED draws them:
#   lowercase: x-height starts at row 6, marks are 3 rows tall at rows 2-4;
#              with a circumflex the tone moves up and to the right (rows 0-2)
#   capitals:  cap height starts at row 3, a lone tone sits in rows 0-2; a
#              circumflex/breve squashes the letter by one row and goes in
#              rows 2-3 with the tone above it in rows 0-1
#   horn:      ơ/ư grow a tick at the top right; Ơ/Ư use a narrower body
#
# usage: tools/mkfont.py [preview.png]

import os, sys, unicodedata

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "src", "vnfont.c")


def load_glyphs(path):
    glyphs = {}
    for block in open(path, encoding="utf-8").read().split("\n\n"):
        lines = [l for l in block.split("\n") if l]
        if not lines or all(l.startswith("# ") for l in lines):
            continue
        name, rows = lines[0].split("  ")[0], lines[1:15]
        assert len(rows) == 14 and all(len(r) == 8 for r in rows), name
        glyphs[name] = [int(r.replace(".", "0").replace("#", "1"), 2) for r in rows]
    return glyphs


G = load_glyphs(os.path.join(HERE, "bked_glyphs.txt"))


def M(*rows):
    return [int(r.replace(".", "0").replace("#", "1"), 2) for r in rows]


def put(g, mark, row, dx=0):
    for i, b in enumerate(mark):
        if b:
            g[row + i] |= (b >> dx) if dx >= 0 else (b << -dx) & 0xFF


def squash(g):
    # Drop the body row that differs least from the one above it
    top = next(i for i in range(14) if g[i])
    bot = max(i for i in range(12) if g[i])
    best = min(range(top + 1, bot + 1), key=lambda i: (bin(g[i] ^ g[i - 1]).count("1"), -i))
    del g[best]
    g.insert(0, 0)


# --- lowercase marks (rows 2-4) ---
L_TONE = {
    "acute": M("....##..", "...##...", "..#....."),
    "grave": M("..##....", "...##...", ".....#.."),
    "hook":  M("..####..", "....##..", "....#..."),
    "tilde": M("........", "..##..##", "##..##.."),
}
L_TONE_DX = {("a", "grave"): -1, ("i", "acute"): 1, ("i", "hook"): 1,
             ("y", "acute"): 1, ("y", "hook"): 1}
L_CIRC = M("....#...", "...###..", "..##.##.")
L_BREVE = M("........", "..#...#.", "...###..")
# tone up and to the right of a circumflex (rows 0-2)
L_CIRC_TONE = {
    "acute": M("......##", ".....##.", "........"),
    "grave": M("....##..", ".....##.", ".......#"),
    "hook":  M("....####", "......##", ".....##."),
    "tilde": M("....##.#", "...#.##.", "........"),
}
# tone centred above a breve (rows 0-1, breve in rows 3-4)
L_BREVE_TONE = {
    "acute": M("....##..", "...##...", "........"),
    "grave": M("..##....", "...##...", "........"),
    "hook":  M("...###..", ".....#..", "....#..."),
    "tilde": M("..##..##", "##..##..", "........"),
}
L_DOT = {"a": "...##...", "u": "...##...", "e": "....##..", "i": "....##..", "o": "....##..", "y": "..##...."}

# --- capital marks ---
U_TONE = {
    "acute": M("....##..", "..##....", "##......"),
    "grave": M(".##.....", "...##...", ".....##."),
    "hook":  M("..####..", "....##..", "...##..."),
    "tilde": M("..#...#.", ".#.#.#..", "#...#..."),
}
U_CIRC = {"O": M("...###..", ".##...##")}
U_CIRC_DEFAULT = M("...##...", ".##..##.")
U_BREVE = M(".#....#.", "..####..")
U_MOD_TONE = {  # rows 0-1, above a circumflex/breve in rows 2-3
    "acute": M("..##....", "##......"),
    "grave": M("...##...", ".....##."),
    "hook":  M("..###...", "....#..."),
    "tilde": M(".##..#..", "#..##..."),
}

# Narrow O/U for the horned capitals, as BKED draws them
NARROW_O = M("........", "........", "........", "...##...", "..####..", ".##..##.", ".##..##.",
             ".##..##.", ".##..##.", ".##..##.", "..####..", "...##...", "........", "........")


def spec(ch):
    if ch in "đĐ":
        return ch, None, None
    d = unicodedata.normalize("NFD", ch)
    mod = tone = None
    for m in d[1:]:
        mod = {"̆": "breve", "̂": "circ", "̛": "horn"}.get(m, mod)
        tone = {"́": "acute", "̀": "grave", "̉": "hook",
                "̃": "tilde", "̣": "dot"}.get(m, tone)
    return d[0], mod, tone


def compose(ch):
    base, mod, tone = spec(ch)
    up = base.isupper()
    b = base.lower()
    if up and mod == "horn":
        if base == "U":
            g = list(G["Ư"])
        else:
            g = list(NARROW_O)
            put(g, M("......##", "......##", "......##"), 2)
        if tone and tone != "dot":
            if base == "U":
                # squash the narrow U under its horn, as BKED's Ữ does
                body = [r & ~0x01 for r in g]
                body[2] = 0
                squash(body)
                g = body
                put(g, M("......##", ".......#", ".......#"), 3)
            put(g, U_TONE[tone], 0)
        if tone == "dot":
            put(g, M("...##..."), 13)
        return g

    g = list(G[base])
    if up:
        if mod in ("circ", "breve"):
            squash(g)
            put(g, (U_CIRC.get(base, U_CIRC_DEFAULT) if mod == "circ" else U_BREVE), 2)
            if tone and tone != "dot":
                put(g, U_MOD_TONE[tone], 0)
        elif tone and tone != "dot":
            put(g, U_TONE[tone], 0)
        if tone == "dot":
            put(g, M("...##..."), 13)
        return g

    if b == "i" and tone and tone != "dot":
        g[3] = g[4] = 0  # dotless
    if mod == "horn":
        put(g, M(".......#", ".......#", ".......#") if b == "u" else M("......##", "......##"), 5)
    if mod == "circ":
        if tone == "hook":
            put(g, L_CIRC, 2, -1)
        else:
            put(g, L_CIRC, 2)
        if tone and tone != "dot":
            put(g, L_CIRC_TONE[tone], 0)
    elif mod == "breve":
        put(g, L_BREVE, 2)
        if tone and tone != "dot":
            put(g, L_BREVE_TONE[tone], 0)
    elif tone and tone != "dot":
        dx = L_TONE_DX.get((b, tone), 0)
        if mod == "horn" and tone == "grave":
            dx = -1
        put(g, L_TONE[tone], 2, dx)
    if tone == "dot":
        put(g, M(L_DOT[b]), 12 if b == "y" else 13)
    return g


VOWELS = "aăâeêioôơuưy"
TONES = ["", "́", "̀", "̉", "̃", "̣"]


def vietnamese_letters():
    out = set()
    for v in VOWELS:
        for t in TONES:
            for c in (v, v.upper()):
                out.add(unicodedata.normalize("NFC", c + t))
    out |= {"đ", "Đ"}
    return sorted(c for c in out if ord(c) > 127)


def build():
    letters = vietnamese_letters()
    font, traced = {}, 0
    for ch in letters:
        if ch in G:
            font[ch] = G[ch]
            traced += 1
        else:
            font[ch] = compose(ch)
    return letters, font, traced


def write_c(letters, font, traced):
    ascii_over = [c for c in sorted(G) if len(c) == 1 and ord(c) < 128]
    L = []
    L.append("// Generated by tools/mkfont.py from tools/bked_glyphs.txt -- do not edit.")
    L.append("// BKED-style 8x14 Vietnamese font: %d letters, %d traced from BKED, the" % (len(letters), traced))
    L.append("// rest composed with BKED's mark placement.")
    L.append("")
    L.append('#include "vnfont.h"')
    L.append("")
    L.append("// Unicode code points of the Vietnamese letters, sorted; index = glyph number")
    L.append("const unsigned short vn_chars[VN_GLYPHS] = {")
    for i in range(0, len(letters), 8):
        L.append("   " + " ".join("0x%04x," % ord(c) for c in letters[i:i + 8]) + "  // " + "".join(letters[i:i + 8]))
    L.append("};")
    L.append("")
    L.append("const unsigned char vn_font[VN_GLYPHS][14] = {")
    for c in letters:
        L.append("   {" + ", ".join("0x%02x" % b for b in font[c]) + "}, // " + c)
    L.append("};")
    L.append("")
    L.append("// ASCII glyphs BKED draws differently from the BIOS font (shifted 1 right, 1 down)")
    L.append("const unsigned char bked_ascii[BKED_ASCII_GLYPHS][15] = {")
    for c in ascii_over:
        L.append("   {0x%02x, " % ord(c) + ", ".join("0x%02x" % b for b in G[c]) + "}, // " + ("'%s'" % c if c != "\\" else "backslash"))
    L.append("};")
    open(OUT, "w", encoding="utf-8").write("\n".join(L) + "\n")
    hdr = os.path.join(HERE, "..", "src", "vnfont.h")
    open(hdr, "w").write("""#ifndef VNFONT_H
#define VNFONT_H

// Generated by tools/mkfont.py -- do not edit.
#define VN_GLYPHS %d
#define BKED_ASCII_GLYPHS %d
#define VN_FONT_HEIGHT 14

extern const unsigned short vn_chars[VN_GLYPHS];
extern const unsigned char vn_font[VN_GLYPHS][14];
extern const unsigned char bked_ascii[BKED_ASCII_GLYPHS][15];

#endif // VNFONT_H
""" % (len(letters), len(ascii_over)))
    return ascii_over


def preview(path, letters, font):
    import zlib, struct
    lines = []
    for t in TONES:
        lines.append([unicodedata.normalize("NFC", v + t) for v in VOWELS] + [" "] +
                     [unicodedata.normalize("NFC", v.upper() + t) for v in VOWELS])
    lines.append(list("đĐ"))
    SC, CW, CH = 4, 9, 14
    W, H = max(len(l) for l in lines) * CW * SC, len(lines) * (CH + 2) * SC
    rows = []
    for py in range(H):
        r, gy = divmod(py // SC, CH + 2)
        row = bytearray()
        for px in range(W):
            c, gx = divmod(px // SC, CW)
            ch = lines[r][c] if c < len(lines[r]) else " "
            g = font.get(ch) or G.get(ch) or [0] * 14
            on = gy < CH and gx < 8 and (g[gy] >> (7 - gx)) & 1
            traced = ch in G
            bg = (0, 0, 170) if traced else (0, 0, 110)
            row += bytes((170, 170, 170) if on else ((40, 40, 60) if gy >= CH else bg))
        rows.append(b"\0" + bytes(row))
    ck = lambda t, b: struct.pack(">I", len(b)) + t + b + struct.pack(">I", zlib.crc32(t + b) & 0xFFFFFFFF)
    open(path, "wb").write(b"\x89PNG\r\n\x1a\n" + ck(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0)) +
                           ck(b"IDAT", zlib.compress(b"".join(rows))) + ck(b"IEND", b""))


if __name__ == "__main__":
    letters, font, traced = build()
    over = write_c(letters, font, traced)
    print("%d Vietnamese letters (%d traced), %d ASCII overrides" % (len(letters), traced, len(over)))
    if len(sys.argv) > 1:
        preview(sys.argv[1], letters, font)
