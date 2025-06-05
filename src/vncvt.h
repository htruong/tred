#ifndef VNCVT_H
#define VNCVT_H

/* TODO: Define more bitmaps for lowercases */

enum tone_mark {
   T_NONE,

   T_UCASE_BREVE,
   T_UCASE_CIRCUMFLEX,

   T_UCASE_ACUTE,
   T_UCASE_GRAVE,
   T_UCASE_HOOK,
   T_UCASE_TILDE,
   T_UCASE_DOT_BELOW,

   T_UCASE_BREVE_ACUTE,
   T_UCASE_BREVE_GRAVE,
   T_UCASE_BREVE_HOOK,
   T_UCASE_BREVE_TILDE,
   T_UCASE_BREVE_DOT_BELOW,

   T_UCASE_CIRCUMFLEX_ACUTE,
   T_UCASE_CIRCUMFLEX_GRAVE,
   T_UCASE_CIRCUMFLEX_HOOK,
   T_UCASE_CIRCUMFLEX_TILDE,
   T_UCASE_CIRCUMFLEX_DOT_BELOW,

   T_UCASE_ATTACHED_HOOK,
   T_UCASE_ATTACHED_HOOK_ACUTE,
   T_UCASE_ATTACHED_HOOK_GRAVE,
   T_UCASE_ATTACHED_HOOK_HOOK,
   T_UCASE_ATTACHED_HOOK_TILDE,
   T_UCASE_ATTACHED_HOOK_DOT_BELOW,

   T_UCASE_D_STROKE,

   T_LCASE_ATTACHED_HOOK,
   T_LCASE_D_STROKE,

   TONES_COUNT
};

int apply_tone_to_char_bitmap(unsigned char __far * dst, unsigned char __far * template, enum tone_mark tone);
unsigned short unicode_to_cp437e(unsigned int wc);
void setup_vietnamese_font(int win2k_workaround);

#define LATIN_EXT_VIETNAMESE_BEGIN 0x1ea0
#define LATIN_EXT_VIETNAMESE_END   0x1ef9

#endif // VNCVT_H