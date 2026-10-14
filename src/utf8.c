// UTF-8 encoding and decoding (BMP only, which covers Vietnamese and CP437)

#include "port.h"
#include "utf8.h"

unsigned utf8_dec(const char *p, const char *e, unsigned *cp) {
   unsigned char c = (unsigned char) p[0];
   unsigned need, i, v;

   if (c < 0x80) {
      *cp = c;
      return 1;
   }
   if ((c & 0xe0) == 0xc0) {
      need = 1;
      v = c & 0x1f;
   } else if ((c & 0xf0) == 0xe0) {
      need = 2;
      v = c & 0x0f;
   } else {
      *cp = UTF8_BAD; // 4-byte sequences are outside the BMP: show as invalid
      return 1;
   }
   if (e - p <= (int) need) {
      *cp = UTF8_BAD;
      return 1;
   }
   for (i = 1; i <= need; i++) {
      if (((unsigned char) p[i] & 0xc0) != 0x80) {
         *cp = UTF8_BAD;
         return 1;
      }
      v = (v << 6) | ((unsigned char) p[i] & 0x3f);
   }
   *cp = v;
   return need + 1;
}

unsigned utf8_enc(unsigned cp, char *out) {
   if (cp < 0x80) {
      out[0] = (char) cp;
      return 1;
   }
   if (cp < 0x800) {
      out[0] = (char) (0xc0 | (cp >> 6));
      out[1] = (char) (0x80 | (cp & 0x3f));
      return 2;
   }
   out[0] = (char) (0xe0 | (cp >> 12));
   out[1] = (char) (0x80 | ((cp >> 6) & 0x3f));
   out[2] = (char) (0x80 | (cp & 0x3f));
   return 3;
}

unsigned utf8_prev(const char *base, unsigned off) {
   unsigned start = off, cp;

   if (off == 0) return 0;
   // Step back over at most 2 continuation bytes, then check it decodes back to off
   do {
      start--;
   } while (start > 0 && off - start < 3 && ((unsigned char) base[start] & 0xc0) == 0x80);
   if (start + utf8_dec(base + start, base + off, &cp) == off) return start;
   return off - 1;
}
