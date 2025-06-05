#include <dos.h>
#include "vgatxt.h"
#include "vncvt.h"

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

void setup_vietnamese_font(int win2k_workaround) {

   unsigned char __far * font_816;
   static unsigned  char __far font_816_alt[256*16];

   //getch();

   //_setvideomode( _TEXTC80 );
   //_clearscreen(_GCLEARSCREEN);

   font_816 = get_fontdef_ptr(6);

   #define UNICODE_TO_EXT_FONTMAP_OFFSET(x) ((x - LATIN_EXT_VIETNAMESE_BEGIN) | 0x80)
   #define EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(x) (x + 0x80 + 0x60)

   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea0)* 16, font_816 + 'A'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea1)* 16, font_816 + 'a'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea2)* 16, font_816 + 'A'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea3)* 16, font_816 + 'a'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea4)* 16, font_816 + 'A'*16, T_UCASE_CIRCUMFLEX_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea5)* 16, font_816 + 'a'*16, T_UCASE_CIRCUMFLEX_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea6)* 16, font_816 + 'A'*16, T_UCASE_CIRCUMFLEX_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea7)* 16, font_816 + 'a'*16, T_UCASE_CIRCUMFLEX_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea8)* 16, font_816 + 'A'*16, T_UCASE_CIRCUMFLEX_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ea9)* 16, font_816 + 'a'*16, T_UCASE_CIRCUMFLEX_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eaa)* 16, font_816 + 'A'*16, T_UCASE_CIRCUMFLEX_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eab)* 16, font_816 + 'a'*16, T_UCASE_CIRCUMFLEX_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eac)* 16, font_816 + 'A'*16, T_UCASE_CIRCUMFLEX_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ead)* 16, font_816 + 'a'*16, T_UCASE_CIRCUMFLEX_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eae)* 16, font_816 + 'A'*16, T_UCASE_BREVE_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eaf)* 16, font_816 + 'a'*16, T_UCASE_BREVE_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb0)* 16, font_816 + 'A'*16, T_UCASE_BREVE_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb1)* 16, font_816 + 'a'*16, T_UCASE_BREVE_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb2)* 16, font_816 + 'A'*16, T_UCASE_BREVE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb3)* 16, font_816 + 'a'*16, T_UCASE_BREVE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb4)* 16, font_816 + 'A'*16, T_UCASE_BREVE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb5)* 16, font_816 + 'a'*16, T_UCASE_BREVE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb6)* 16, font_816 + 'A'*16, T_UCASE_BREVE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb7)* 16, font_816 + 'a'*16, T_UCASE_BREVE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb8)* 16, font_816 + 'E'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eb9)* 16, font_816 + 'e'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eba)* 16, font_816 + 'E'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ebb)* 16, font_816 + 'e'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ebc)* 16, font_816 + 'E'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ebd)* 16, font_816 + 'e'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ebe)* 16, font_816 + 'E'*16, T_UCASE_CIRCUMFLEX_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ebf)* 16, font_816 + 'e'*16, T_UCASE_CIRCUMFLEX_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec0)* 16, font_816 + 'E'*16, T_UCASE_CIRCUMFLEX_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec1)* 16, font_816 + 'e'*16, T_UCASE_CIRCUMFLEX_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec2)* 16, font_816 + 'E'*16, T_UCASE_CIRCUMFLEX_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec3)* 16, font_816 + 'e'*16, T_UCASE_CIRCUMFLEX_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec4)* 16, font_816 + 'E'*16, T_UCASE_CIRCUMFLEX_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec5)* 16, font_816 + 'e'*16, T_UCASE_CIRCUMFLEX_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec6)* 16, font_816 + 'E'*16, T_UCASE_CIRCUMFLEX_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec7)* 16, font_816 + 'e'*16, T_UCASE_CIRCUMFLEX_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec8)* 16, font_816 + 'I'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ec9)* 16, font_816 + 'i'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eca)* 16, font_816 + 'I'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ecb)* 16, font_816 + 'i'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ecc)* 16, font_816 + 'O'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ecd)* 16, font_816 + 'o'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ece)* 16, font_816 + 'O'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ecf)* 16, font_816 + 'o'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed0)* 16, font_816 + 'O'*16, T_UCASE_CIRCUMFLEX_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed1)* 16, font_816 + 'o'*16, T_UCASE_CIRCUMFLEX_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed2)* 16, font_816 + 'O'*16, T_UCASE_CIRCUMFLEX_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed3)* 16, font_816 + 'o'*16, T_UCASE_CIRCUMFLEX_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed4)* 16, font_816 + 'O'*16, T_UCASE_CIRCUMFLEX_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed5)* 16, font_816 + 'o'*16, T_UCASE_CIRCUMFLEX_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed6)* 16, font_816 + 'O'*16, T_UCASE_CIRCUMFLEX_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed7)* 16, font_816 + 'o'*16, T_UCASE_CIRCUMFLEX_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed8)* 16, font_816 + 'O'*16, T_UCASE_CIRCUMFLEX_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ed9)* 16, font_816 + 'o'*16, T_UCASE_CIRCUMFLEX_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eda)* 16, font_816 + 'O'*16, T_UCASE_ATTACHED_HOOK_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1edb)* 16, font_816 + 'o'*16, T_UCASE_ATTACHED_HOOK_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1edc)* 16, font_816 + 'O'*16, T_UCASE_ATTACHED_HOOK_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1edd)* 16, font_816 + 'o'*16, T_UCASE_ATTACHED_HOOK_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ede)* 16, font_816 + 'O'*16, T_UCASE_ATTACHED_HOOK_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1edf)* 16, font_816 + 'o'*16, T_UCASE_ATTACHED_HOOK_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee0)* 16, font_816 + 'O'*16, T_UCASE_ATTACHED_HOOK_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee1)* 16, font_816 + 'o'*16, T_UCASE_ATTACHED_HOOK_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee2)* 16, font_816 + 'O'*16, T_UCASE_ATTACHED_HOOK_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee3)* 16, font_816 + 'o'*16, T_UCASE_ATTACHED_HOOK_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee4)* 16, font_816 + 'U'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee5)* 16, font_816 + 'u'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee6)* 16, font_816 + 'U'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee7)* 16, font_816 + 'u'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee8)* 16, font_816 + 'U'*16, T_UCASE_ATTACHED_HOOK_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ee9)* 16, font_816 + 'u'*16, T_UCASE_ATTACHED_HOOK_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eea)* 16, font_816 + 'U'*16, T_UCASE_ATTACHED_HOOK_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eeb)* 16, font_816 + 'u'*16, T_UCASE_ATTACHED_HOOK_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eec)* 16, font_816 + 'U'*16, T_UCASE_ATTACHED_HOOK_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eed)* 16, font_816 + 'u'*16, T_UCASE_ATTACHED_HOOK_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eee)* 16, font_816 + 'U'*16, T_UCASE_ATTACHED_HOOK_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1eef)* 16, font_816 + 'u'*16, T_UCASE_ATTACHED_HOOK_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef0)* 16, font_816 + 'U'*16, T_UCASE_ATTACHED_HOOK_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef1)* 16, font_816 + 'u'*16, T_UCASE_ATTACHED_HOOK_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef2)* 16, font_816 + 'Y'*16, T_UCASE_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef3)* 16, font_816 + 'y'*16, T_UCASE_GRAVE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef4)* 16, font_816 + 'Y'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef5)* 16, font_816 + 'y'*16, T_UCASE_DOT_BELOW);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef6)* 16, font_816 + 'Y'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef7)* 16, font_816 + 'y'*16, T_UCASE_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef8)* 16, font_816 + 'Y'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + UNICODE_TO_EXT_FONTMAP_OFFSET(0x1ef9)* 16, font_816 + 'y'*16, T_UCASE_TILDE);
   
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(0)* 16, font_816 + 'A'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(1)* 16, font_816 + 'a'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(2)* 16, font_816 + 'O'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(3)* 16, font_816 + 'o'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(4)* 16, font_816 + 'A'*16, T_UCASE_BREVE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(5)* 16, font_816 + 'a'*16, T_UCASE_BREVE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(6)* 16, font_816 + 'D'*16, T_UCASE_D_STROKE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(7)* 16, font_816 + 'd'*16, T_LCASE_D_STROKE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(8)* 16, font_816 + 'I'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(9)* 16, font_816 + 'i'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(10)* 16, font_816 + 'U'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(11)* 16, font_816 + 'u'*16, T_UCASE_TILDE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(12)* 16, font_816 + 'O'*16, T_UCASE_ATTACHED_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(13)* 16, font_816 + 'o'*16, T_LCASE_ATTACHED_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(14)* 16, font_816 + 'U'*16, T_UCASE_ATTACHED_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(15)* 16, font_816 + 'u'*16, T_LCASE_ATTACHED_HOOK);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(16)* 16, font_816 + 'E'*16, T_UCASE_CIRCUMFLEX);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(17)* 16, font_816 + 'Y'*16, T_UCASE_ACUTE);
   apply_tone_to_char_bitmap(font_816_alt + EXTRA_VIETNAMESE_TO_EXT_FONTMAP_OFFSET(18)* 16, font_816 + 'y'*16, T_UCASE_ACUTE);


   //install_fontdef(0, 16, 256, 0, font_816);
   install_fontdef(1, 16, 256, 0, font_816_alt);
   disable_color_plane_3();
   // 0x04 = use font block 1 when displaying high intensity font
   activate_fontblock(0x04);

}
