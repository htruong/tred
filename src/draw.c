// Clipped drawing primitives on a viewport of the surface

#include <string.h>
#include "surface.h"
#include "draw.h"
#include "utf8.h"
#include "vncvt.h"

void v_put(const view_t *v, int c, int r, unsigned short code, unsigned char a) {
   if (c < 0 || r < 0 || c >= v->w || r >= v->h) return;
   scr_put(v->x + c, v->y + r, code, a);
}

void v_fill(const view_t *v, int c, int r, int w, int h, unsigned short code, unsigned char a) {
   if (c < 0) { w += c; c = 0; }
   if (r < 0) { h += r; r = 0; }
   if (c + w > v->w) w = v->w - c;
   if (r + h > v->h) h = v->h - r;
   if (w > 0 && h > 0) scr_fill(v->x + c, v->y + r, w, h, code, a);
}

int v_text(const view_t *v, int c, int r, const char *ascii, unsigned char a) {
   int n = 0;

   for (; *ascii; ascii++, n++) v_put(v, c + n, r, (unsigned char) *ascii, a);
   return n;
}

int v_utf8(const view_t *v, int c, int r, const char *s, unsigned char a) {
   const char *e = s + strlen(s);
   unsigned cp;
   int n = 0;

   while (s < e) {
      s += utf8_dec(s, e, &cp);
      v_put(v, c + n++, r, unicode_to_cp437e(cp), a);
   }
   return n;
}

void v_frame(const view_t *v, unsigned char a) {
   v_put(v, 0, 0, 0xda, a);
   v_fill(v, 1, 0, v->w - 2, 1, 0xc4, a);
   v_put(v, v->w - 1, 0, 0xbf, a);
   v_fill(v, 0, 1, 1, v->h - 2, 0xb3, a);
   v_fill(v, v->w - 1, 1, 1, v->h - 2, 0xb3, a);
   v_put(v, 0, v->h - 1, 0xc0, a);
   v_fill(v, 1, v->h - 1, v->w - 2, 1, 0xc4, a);
   v_put(v, v->w - 1, v->h - 1, 0xd9, a);
}
