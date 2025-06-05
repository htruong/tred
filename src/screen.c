// Screen and buffer management functions for displaying text on the screen

#include <dos.h>
#include "screen.h"

#define VIDEO_SEG 0xB800
char __far *video = (char __far *)MK_FP(VIDEO_SEG, 0);

void write_char_at(unsigned char row, unsigned char col, char ch, char attr) {
   unsigned short vidAddr = (row * 160) + (col * 2);
   video[vidAddr] = ch;         // character
   video[vidAddr + 1] = attr;   // attribute
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

void blitstring(unsigned short __far * s) {
   char __far *buf = video;
   unsigned short s_cur;
   unsigned short buf_cur;
   unsigned short cur_wc;
   for (s_cur = 0; s_cur < SCREEN_WIDTH * SCREEN_HEIGHT; s_cur++) {
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
}