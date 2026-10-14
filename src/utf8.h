#ifndef UTF8_H
#define UTF8_H

#define UTF8_BAD 0xfffd

// Decode one code point at p (p < e). Returns its byte length; an invalid
// sequence decodes as UTF8_BAD with length 1 so the bytes round-trip.
unsigned utf8_dec(const char *p, const char *e, unsigned *cp);
// Encode cp into out (up to 3 bytes for the BMP); returns the byte length.
unsigned utf8_enc(unsigned cp, char *out);
// Offset of the code point that ends at off.
unsigned utf8_prev(const char *base, unsigned off);

#endif // UTF8_H
