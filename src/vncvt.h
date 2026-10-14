#ifndef VNCVT_H
#define VNCVT_H

// Codes above 0xff are glyphs in the second font (bright attribute); the low
// byte is the glyph's slot there. Vietnamese letter n of vn_chars[] lives in
// slot 0x80 + n, and the few past 128 in slots 0x01.. (control chars are free).
#define VN_SLOT(n) ((n) < 128 ? 0x80 + (n) : (n) - 127)
#define VN_CODE(n) (0x100 | VN_SLOT(n))

unsigned short unicode_to_cp437e(unsigned int wc);

#endif // VNCVT_H
