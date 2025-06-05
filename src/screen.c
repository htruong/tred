// Screen and buffer management functions for displaying text on the screen

#include <dos.h>
#include "screen.h"

// SLOW
void puttextxy(unsigned char __far * s, unsigned char textattr, unsigned short len, unsigned char row, unsigned char col) {
   union REGPACK r;
   
   r.x.ax = 0x1301;
   r.h.bh = 0;
   r.h.bl = textattr;
   r.x.cx = len;
   r.h.dh = row;
   r.h.dl = col;
   r.w.es = FP_SEG(s);
   r.w.bp = FP_OFF(s);

   intr(0x10, &r);
}

// SLOW
void puttext(unsigned char __far * s, unsigned char textattr, unsigned short len) {
   union REGPACK r;
   unsigned char row, col;

   getxy(&row, &col);
   puttextxy(s, textattr, len, row, col);
}

void putcharattrxy(unsigned char __far * s, unsigned short len, unsigned char row, unsigned char col) {
   union REGPACK r;
   
   r.x.ax = 0x1302;
   r.h.bh = 0;
   r.x.cx = len;
   r.h.dh = row;
   r.h.dl = col;
   r.w.es = FP_SEG(s);
   r.w.bp = FP_OFF(s);

   intr(0x10, &r);
}

// len is half of len of s
void putcharattr(unsigned char __far * s, unsigned short len) {
   union REGPACK r;
   unsigned char row, col;

   getxy(&row, &col);
   putcharattrxy(s, len, row, col);
}

void getxy(unsigned char * row, unsigned char * col) {
   union REGPACK r;

   r.h.ah = 0x03;
   r.h.bh = 0x00;

   intr(0x10, &r);

   *row = r.h.dh;
   *col = r.h.dl;

   //printf("Current cursor position: row %d col %d\n", *row, *col);
}

void puttexte(unsigned short __far * s, unsigned short len) {
   static char buf[SCREEN_WIDTH * SCREEN_HEIGHT * 2];
   unsigned short s_cur;
   unsigned short buf_cur;
   unsigned short cur_wc;
   s_cur = 0;
   buf_cur = 0;

   for (s_cur = 0; s_cur < len; s_cur++) {
      if (s_cur > SCREEN_WIDTH * SCREEN_HEIGHT) {
         break;
      }
      cur_wc = s[s_cur];
      buf_cur = s_cur + s_cur;
      if (cur_wc > 0xff) {
         buf[buf_cur] = (char) ( cur_wc & 0xff ) ;
         buf[buf_cur + 1] = 0x0f;
      } else {
         buf[buf_cur] = (char) cur_wc;
         buf[buf_cur + 1] = 0x07;
      }
   }

   putcharattr(buf, s_cur);
}