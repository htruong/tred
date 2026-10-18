#!/usr/bin/env python3
# Renders a tools/vgadump.c dump (TEXT.BIN, FONT.BIN, REGS.TXT in DIR) to a PNG,
# and with --text prints the screen's ASCII characters.
# usage: vgarender.py DIR out.png [scale]  |  vgarender.py DIR --text
import struct, sys, zlib

d = sys.argv[1]
text = open(d + "/TEXT.BIN", "rb").read()
if sys.argv[2] == "--regs":
    v = list(map(int, open(d + "/REGS.TXT").read().split()))
    print("char height %d, cursor lines %d-%d%s, at %d" % (v[2], v[19] & 31, v[20] & 31, " (hidden)" if v[19] & 0x20 else "", v[21] + v[22]))
    sys.exit(0)
if sys.argv[2] == "--text":
    for r in range(25):
        print("".join(chr(text[(r * 80 + c) * 2]) if 32 <= text[(r * 80 + c) * 2] < 127 else "." for c in range(80)).rstrip())
    sys.exit(0)
font = open(d + "/FONT.BIN", "rb").read()
v = list(map(int, open(d + "/REGS.TXT").read().split()))
cms, pe, ch = v[0], v[1], v[2]
cstart, cend = v[19], v[20]
cursor = -1 if cstart & 0x20 else v[21] + v[22]
OFF = [0, 0x4000, 0x8000, 0xC000, 0x2000, 0x6000, 0xA000, 0xE000]
map_a = ((cms >> 2) & 3) | ((cms >> 3) & 4)  # attribute bit 3 set
map_b = (cms & 3) | ((cms >> 2) & 4)
ega = lambda x: (((x >> 2) & 1) * 0xAA + ((x >> 5) & 1) * 0x55,
                 ((x >> 1) & 1) * 0xAA + ((x >> 4) & 1) * 0x55,
                 (x & 1) * 0xAA + ((x >> 3) & 1) * 0x55)
pal = [ega(x) for x in v[3:19]]
sc = int(sys.argv[3]) if len(sys.argv) > 3 else 1
W, H = 640 * sc, 25 * ch * sc
rows = []
for py in range(H):
    r, gy = divmod(py // sc, ch)
    row = bytearray()
    for px in range(0, W, sc):
        c, gx = divmod(px // sc, 8)
        code, at = text[(r * 80 + c) * 2], text[(r * 80 + c) * 2 + 1]
        bits = font[OFF[map_a if at & 8 else map_b] + code * 32 + gy]
        on = (bits >> (7 - gx)) & 1 or (r * 80 + c == cursor and (cstart & 31) <= gy <= (cend & 31))
        row += bytes(pal[(at & 15) & pe] if on else pal[(at >> 4) & 7]) * sc
    rows.append(b"\0" + bytes(row))
chunk = lambda t, b: struct.pack(">I", len(b)) + t + b + struct.pack(">I", zlib.crc32(t + b) & 0xFFFFFFFF)
open(sys.argv[2], "wb").write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0)) +
                               chunk(b"IDAT", zlib.compress(b"".join(rows), 9)) + chunk(b"IEND", b""))
