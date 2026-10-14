// Sets up a custom font for displaying Vietnamese characters in a text mode
// on x86 VGA using BIOS interrupts.

#include <dos.h>
#include "vgatxt.h"
#include "vncvt.h"
#include "vnfont.h"

void __far * get_fontdef_ptr(unsigned char font_block) {
   union REGPACK r;

   r.x.ax = 0x1130;
   r.h.bh = font_block;

   intr(0x10, &r);
   //printf("Returned height: %d, rows: %d, es=%x, bp=%x, should return 0x%x\n", r.x.cx, r.h.dl, r.w.es, r.w.bp, MK_FP(r.w.es, r.w.bp));
   return MK_FP(r.w.es, r.w.bp);
}

void install_fontdef(unsigned char font_block, unsigned char font_height, unsigned short num_chars, unsigned char start_char, void __far * ptr) {
   union REGPACK r;

   r.x.ax = 0x1100;
   r.h.bh = font_height;
   r.h.bl = font_block;
   r.x.cx = num_chars;
   r.x.dx = start_char;
   r.w.es = FP_SEG(ptr);
   r.w.bp = FP_OFF(ptr);

   intr(0x10, &r);
}

void set_char(unsigned char font_height, unsigned char fontblock, unsigned char ascii_char, void __far * ptr) {
   union REGPACK r;

   r.x.ax = 0x1100;
   r.h.bh = font_height;
   r.h.bl = fontblock;
   r.w.cx = 1;
   r.w.dx = ascii_char;
   r.w.es = FP_SEG(ptr);
   r.w.bp = FP_OFF(ptr);

   intr(0x10, &r);
}

void activate_fontblock(unsigned char font_block) {
   union REGPACK r;

   r.x.ax = 0x1103;
   r.h.bl = font_block;

   intr(0x10, &r);
}

void disable_color_plane_3() {
   union REGPACK r;

   r.x.ax = 0x1000;
   r.h.bl = 0x12;
   r.h.bh = 0x07;

   intr(0x10, &r);
}


void set_scan_lines(unsigned char setting) {
   union REGPACK r;

   r.h.ah = 0x12;
   r.h.al = setting;
   r.h.bl = 0x30;
   intr(0x10, &r);

   r.x.ax = 0x0003;
   intr(0x10, &r);
}

void restore_text_mode(void) {
   set_scan_lines(SCAN_LINES_400);
}

// Like BKED: 350-line 80x25 text mode with an 8x14 font. Block 0 is the BIOS
// 8x14 font moved 1 pixel right and 1 down (which leaves 5 rows above the
// lowercase letters for diacritics) with BKED's redrawn letters on top; block 1
// is a copy of it plus the Vietnamese letters, shown with the bright attribute.
void setup_vietnamese_font(int win2k_workaround) {
   static unsigned char __far font0[256 * VN_FONT_HEIGHT];
   static unsigned char __far font1[256 * VN_FONT_HEIGHT];
   unsigned char __far * bios;
   unsigned char __far * g;
   union REGPACK r;
   int c, i;

   set_scan_lines(SCAN_LINES_350);
   bios = get_fontdef_ptr(2);

   for (c = 0; c < 256; c++) {
      g = font0 + c * VN_FONT_HEIGHT;
      if (c < 0x20 || (c >= 0xb0 && c <= 0xdf)) {
         // Symbols and line drawing must keep touching the cell edges
         for (i = 0; i < VN_FONT_HEIGHT; i++) g[i] = bios[c * VN_FONT_HEIGHT + i];
      } else {
         g[0] = 0;
         for (i = 1; i < VN_FONT_HEIGHT; i++) g[i] = bios[c * VN_FONT_HEIGHT + i - 1] >> 1;
      }
   }
   for (i = 0; i < BKED_ASCII_GLYPHS; i++) {
      g = font0 + bked_ascii[i][0] * VN_FONT_HEIGHT;
      for (c = 0; c < VN_FONT_HEIGHT; c++) g[c] = bked_ascii[i][c + 1];
   }

   for (i = 0; i < 256 * VN_FONT_HEIGHT; i++) font1[i] = font0[i];
   for (i = 0; i < VN_GLYPHS; i++) {
      g = font1 + VN_SLOT(i) * VN_FONT_HEIGHT;
      for (c = 0; c < VN_FONT_HEIGHT; c++) g[c] = vn_font[i][c];
   }

   // AX=1110h also reprograms the CRTC for 14-line characters (25 rows)
   r.x.ax = 0x1110;
   r.h.bh = VN_FONT_HEIGHT;
   r.h.bl = 0;
   r.x.cx = 256;
   r.x.dx = 0;
   r.w.es = FP_SEG(font0);
   r.w.bp = FP_OFF(font0);
   intr(0x10, &r);

   install_fontdef(1, VN_FONT_HEIGHT, 256, 0, font1);
   disable_color_plane_3();
   // 0x04 = use font block 1 when displaying high intensity font
   activate_fontblock(0x04);
}
