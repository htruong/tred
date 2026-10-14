// Shadow-buffered text screen with per-row dirty spans

#include <dos.h>
#include <stdlib.h>
#include "surface.h"

static cell_t back[SCR_W * SCR_H];
static cell_t front[SCR_W * SCR_H];
static unsigned char dlo[SCR_H], dhi[SCR_H];
static cell_t __far *vid = (cell_t __far *) MK_FP(0xB800, 0);
unsigned long scr_writes;


void scr_invalidate(void) {
   int i;

   for (i = 0; i < SCR_W * SCR_H; i++) front[i] = 0xffff;
   for (i = 0; i < SCR_H; i++) {
      dlo[i] = 0;
      dhi[i] = SCR_W - 1;
   }
}

void scr_init(void) {
   int i;

   for (i = 0; i < SCR_W * SCR_H; i++) back[i] = MKCELL(' ', 0x07);
   scr_invalidate();
}

static void set(int x, int y, cell_t c) {
   cell_t *b = &back[y * SCR_W + x];

   if (*b == c) return;
   *b = c;
   if (dlo[y] > dhi[y]) {
      dlo[y] = dhi[y] = (unsigned char) x;
   } else {
      if (x < dlo[y]) dlo[y] = (unsigned char) x;
      if (x > dhi[y]) dhi[y] = (unsigned char) x;
   }
}

void scr_put(int x, int y, unsigned short code, unsigned char attr) {
   if (x < 0 || y < 0 || x >= SCR_W || y >= SCR_H) return;
   set(x, y, MKCELL(code, attr));
}

void scr_put_cells(int x, int y, const cell_t *c, int n) {
   int i;

   if (y < 0 || y >= SCR_H) return;
   for (i = 0; i < n; i++) {
      if (x + i >= 0 && x + i < SCR_W) set(x + i, y, c[i]);
   }
}

void scr_fill(int x, int y, int w, int h, unsigned short code, unsigned char attr) {
   int i, j;
   cell_t c = MKCELL(code, attr);

   for (j = y; j < y + h; j++) {
      for (i = x; i < x + w; i++) {
         if (i >= 0 && j >= 0 && i < SCR_W && j < SCR_H) set(i, j, c);
      }
   }
}

cell_t scr_get(int x, int y) {
   return back[y * SCR_W + x];
}

void scr_shade(int x, int y, int w, int h, unsigned char attr) {
   int i, j;
   cell_t c;

   for (j = y; j < y + h; j++) {
      for (i = x; i < x + w; i++) {
         if (i < 0 || j < 0 || i >= SCR_W || j >= SCR_H) continue;
         c = back[j * SCR_W + i];
         set(i, j, MKCELL(CELL_CODE(c), attr));
      }
   }
}

void scr_flush(void) {
   int y, x, i;

   for (y = 0; y < SCR_H; y++) {
      if (dlo[y] > dhi[y]) continue;
      for (x = dlo[y]; x <= dhi[y]; x++) {
         i = y * SCR_W + x;
         if (back[i] != front[i]) {
            vid[i] = front[i] = back[i];
            scr_writes++;
         }
      }
      dlo[y] = 1;
      dhi[y] = 0; // empty span
   }
}

cell_t *scr_save(int x, int y, int w, int h) {
   cell_t *s = (cell_t *) malloc(w * h * sizeof(cell_t));
   int i, j;

   if (!s) return 0;
   for (j = 0; j < h; j++) {
      for (i = 0; i < w; i++) {
         s[j * w + i] = (x + i < SCR_W && y + j < SCR_H) ? back[(y + j) * SCR_W + x + i] : 0;
      }
   }
   return s;
}

void scr_restore(int x, int y, int w, int h, cell_t *saved) {
   int i, j;

   if (!saved) return;
   for (j = 0; j < h; j++) {
      for (i = 0; i < w; i++) {
         if (x + i < SCR_W && y + j < SCR_H) set(x + i, y + j, saved[j * w + i]);
      }
   }
   free(saved);
}
