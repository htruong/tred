#include "screen.h"

void puttext(unsigned char __far * s, unsigned char textattr, unsigned short len) {
   union REGPACK r;
   unsigned char row, col;

   //printf("Putting string %s attr %x len %d at col %d row %d\n", s, textattr, len, row, col);

   //return;
   r.h.ah = 0x03;
   r.h.bh = 0x00;

   intr(0x10, &r);

   row = r.h.dh;
   col = r.h.dl;

   memset( &r, 0, sizeof( r ) );
   
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

void puttexte(unsigned short __far * s, unsigned short len) {
   static char buf[80*25 + 1];
   unsigned short buf_cur;
   unsigned short s_cur;
   enum display_fontslot slot;
   slot = SLOT_UNKNOWN;
   s_cur = 0;
   buf_cur = 0;
   //printf("Putting extended string len %d\n", len);

   while (s_cur < len) {
      if (buf_cur > 80*25) {
         if (slot == SLOT_NORMAL) {
            buf[buf_cur] = '\0';
            puttext(buf, 0x07, buf_cur);
            buf_cur = 0;
         } else {
            buf[buf_cur] = '\0';
            puttext(buf, 0x0f, buf_cur);
            buf_cur = 0;
         }
      }

      if (s[s_cur] > 0xff) {
         if (slot == SLOT_NORMAL) {
            buf[buf_cur] = '\0';
            puttext(buf, 0x07, buf_cur);
            buf_cur = 0;
         }
         slot = SLOT_EXTENDED;
      } else {
         if (slot == SLOT_EXTENDED) {
            buf[buf_cur] = '\0';
            puttext(buf, 0x0f, buf_cur);
            buf_cur = 0;
         }
         slot = SLOT_NORMAL;
      }
      buf[buf_cur] = (char) ( s[s_cur] & 0xff ) ;
      buf_cur++;
      s_cur++;
   }

   if (buf_cur > 0) {
      if (slot == SLOT_NORMAL) {
         buf[buf_cur] = '\0';
         puttext(buf, 0x07, buf_cur);
      } else {
         buf[buf_cur] = '\0';
         puttext(buf, 0x0f, buf_cur);
      }
   }
}