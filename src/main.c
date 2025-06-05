#include <dos.h>
#include <stdio.h>
#include <graph.h>
#include <string.h>

#include "screen.h"

enum p_state {
   P_NORMAL,
   P_EXPECT_ONE_CONT_B,
   P_EXPECT_TWO_CONT_B,
   P_EXPECT_THREE_CONT_B,
   P_INVALID
};

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

int apply_tone_to_char_bitmap(unsigned char __far * dst, unsigned char __far * template, enum tone_mark tone) {
   static char tones_bitmap[TONES_COUNT][16] = {
      /* NONE     */  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

      /* UBREVE   */  {0x6C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UCRFLX   */  {0x38, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

      /* UACUTE   */  {0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UGRAVE   */  {0x0C, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UHOOK    */  {0x06, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UTILDE   */  {0x14, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UDTBLW   */  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
      
      /* UBACUTE  */  {0x6D, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UBGRAVE  */  {0x2A, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UBHOOK   */  {0x2B, 0x2A, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UBTILDE  */  {0x6D, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UBDTBLW  */  {0x0C, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},

      /* UCACUTE  */  {0x11, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UCGRAVE  */  {0x3A, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UCHOOK   */  {0x3B, 0x6D, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UCTILDE  */  {0x3D, 0x6A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UCDTBLW  */  {0x38, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},

      /* UAHOOK   */  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UAACUTE  */  {0x18, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UAGRAVE  */  {0x18, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UAHOOK   */  {0x18, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UATILDE  */  {0x14, 0x29, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* UADTBLW  */  {0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},

      /* UDSTROK  */  {0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

      /* LAHOOK   */  {0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      /* LDSTROK  */  {0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
   };

   int i;

   for (i = 0; i < 16; i++) {
      dst[i] = template[i] | tones_bitmap[tone][i];
   }
}

// Lower chars regions of the Unicode characters that needs remapping to cp437
#define LOW_CHARS_END 0xff
#define VIETNAMESE_ADDITIONAL_MAPPING_BEGIN  0x60
#define VIETNAMESE_ADDITIONAL_MAPPING_OFFSET 0x180
#define TO_VIETNAMESE_OFFSET(x) (x + VIETNAMESE_ADDITIONAL_MAPPING_OFFSET)
#define BOX_DRAW_BEGIN 0x2500
#define BOX_DRAW_END   0x25ff
#define LATIN_EXT_VIETNAMESE_BEGIN 0x1ea0
#define LATIN_EXT_VIETNAMESE_END   0x1ef9

unsigned short unicode_to_cp437e(unsigned int wc) {

   static unsigned short low_chars[] = {
      0x00 /*   0000*/, 0xfe /*   0001*/, 0xfe /*   0002*/, 0xfe /*   0003*/, 0xfe /*   0004*/, 0xfe /*   0005*/, 0xfe /*   0006*/, 0xfe /*   0007*/, 
      0xfe /*   0008*/, 0xfe /*   0009*/, 0xfe /*   000a*/, 0x0a /*   000b*/, 0xfe /*   000c*/, 0xfe /*   000d*/, 0x0d /*   000e*/, 0xfe /*   000f*/, 
      0xfe /*   0010*/, 0xfe /*   0011*/, 0xfe /*   0012*/, 0xfe /*   0013*/, 0xfe /*   0014*/, 0xfe /*   0015*/, 0xfe /*   0016*/, 0xfe /*   0017*/, 
      0xfe /*   0018*/, 0xfe /*   0019*/, 0xfe /*   001a*/, 0xfe /*   001b*/, 0xfe /*   001c*/, 0xfe /*   001d*/, 0xfe /*   001e*/, 0xfe /*   001f*/, 
      0x20 /*   0020*/, 0x21 /* ! 0021*/, 0x22 /* " 0022*/, 0x23 /* # 0023*/, 0x24 /* $ 0024*/, 0x25 /* % 0025*/, 0x26 /* & 0026*/, 0x27 /* ' 0027*/, 
      0x28 /* ( 0028*/, 0x29 /* ) 0029*/, 0x2a /* * 002a*/, 0x2b /* + 002b*/, 0x2c /* , 002c*/, 0x2d /* - 002d*/, 0x2e /* . 002e*/, 0x2f /* / 002f*/, 
      0x30 /* 0 0030*/, 0x31 /* 1 0031*/, 0x32 /* 2 0032*/, 0x33 /* 3 0033*/, 0x34 /* 4 0034*/, 0x35 /* 5 0035*/, 0x36 /* 6 0036*/, 0x37 /* 7 0037*/, 
      0x38 /* 8 0038*/, 0x39 /* 9 0039*/, 0x3a /* : 003a*/, 0x3b /* ; 003b*/, 0x3c /* < 003c*/, 0x3d /* = 003d*/, 0x3e /* > 003e*/, 0x3f /* ? 003f*/, 
      0x40 /* @ 0040*/, 0x41 /* A 0041*/, 0x42 /* B 0042*/, 0x43 /* C 0043*/, 0x44 /* D 0044*/, 0x45 /* E 0045*/, 0x46 /* F 0046*/, 0x47 /* G 0047*/, 
      0x48 /* H 0048*/, 0x49 /* I 0049*/, 0x4a /* J 004a*/, 0x4b /* K 004b*/, 0x4c /* L 004c*/, 0x4d /* M 004d*/, 0x4e /* N 004e*/, 0x4f /* O 004f*/, 
      0x50 /* P 0050*/, 0x51 /* Q 0051*/, 0x52 /* R 0052*/, 0x53 /* S 0053*/, 0x54 /* T 0054*/, 0x55 /* U 0055*/, 0x56 /* V 0056*/, 0x57 /* W 0057*/, 
      0x58 /* X 0058*/, 0x59 /* Y 0059*/, 0x5a /* Z 005a*/, 0x5b /* [ 005b*/, 0x5c /* \ 005c*/, 0x5d /* ] 005d*/, 0x5e /* ^ 005e*/, 0x5f /* _ 005f*/, 
      0x60 /* ` 0060*/, 0x61 /* a 0061*/, 0x62 /* b 0062*/, 0x63 /* c 0063*/, 0x64 /* d 0064*/, 0x65 /* e 0065*/, 0x66 /* f 0066*/, 0x67 /* g 0067*/, 
      0x68 /* h 0068*/, 0x69 /* i 0069*/, 0x6a /* j 006a*/, 0x6b /* k 006b*/, 0x6c /* l 006c*/, 0x6d /* m 006d*/, 0x6e /* n 006e*/, 0x6f /* o 006f*/, 
      0x70 /* p 0070*/, 0x71 /* q 0071*/, 0x72 /* r 0072*/, 0x73 /* s 0073*/, 0x74 /* t 0074*/, 0x75 /* u 0075*/, 0x76 /* v 0076*/, 0x77 /* w 0077*/, 
      0x78 /* x 0078*/, 0x79 /* y 0079*/, 0x7a /* z 007a*/, 0x7b /* { 007b*/, 0x7c /* | 007c*/, 0x7d /* } 007d*/, 0x7e /* ~ 007e*/, 0xfe /*   007f*/, 
      0xfe /*   0080*/, 0xfe /*   0081*/, 0xfe /*   0082*/, 0xfe /*   0083*/, 0xfe /*   0084*/, 0xfe /*   0085*/, 0xfe /*   0086*/, 0xfe /*   0087*/, 
      0xfe /*   0088*/, 0xfe /*   0089*/, 0xfe /*   008a*/, 0xfe /*   008b*/, 0xfe /*   008c*/, 0xfe /*   008d*/, 0xfe /*   008e*/, 0xfe /*   008f*/, 
      0xfe /*   0090*/, 0xfe /*   0091*/, 0xfe /*   0092*/, 0xfe /*   0093*/, 0xfe /*   0094*/, 0xfe /*   0095*/, 0xfe /*   0096*/, 0xfe /*   0097*/, 
      0xfe /*   0098*/, 0xfe /*   0099*/, 0xfe /*   009a*/, 0xfe /*   009b*/, 0xfe /*   009c*/, 0xfe /*   009d*/, 0xfe /*   009e*/, 0xfe /*   009f*/, 
      0xff /*   00a0*/, 0xad /* ¡ 00a1*/, 0x9b /* ¢ 00a2*/, 0x9c /* £ 00a3*/, 0xfe /*   00a4*/, 0x9d /* ¥ 00a5*/, 0xfe /*   00a6*/, 0x15 /* § 00a7*/, 
      0xfe /*   00a8*/, 0xfe /*   00a9*/, 0xa6 /* ª 00aa*/, 0xae /* « 00ab*/, 0xaa /* ¬ 00ac*/, 0xfe /*   00ad*/, 0xfe /*   00ae*/, 0xfe /*   00af*/, 
      0xf8 /* ° 00b0*/, 0xf1 /* ± 00b1*/, 0xfd /* ² 00b2*/, 0xfe /*   00b3*/, 0xfe /*   00b4*/, 0xe6 /* µ 00b5*/, 0x14 /* ¶ 00b6*/, 0xfa /* · 00b7*/, 
      0xfe /*   00b8*/, 0xfe /*   00b9*/, 0xa7 /* º 00ba*/, 0xaf /* » 00bb*/, 0xac /* ¼ 00bc*/, 0xab /* ½ 00bd*/, 0xfe /*   00be*/, 0xa8 /* ¿ 00bf*/, 
      0xfe /*   00c0*/, 0xfe /*   00c1*/, 0xfe /*   00c2*/, 0xfe /*   00c3*/, 0x8e /* Ä 00c4*/, 0x8f /* Å 00c5*/, 0x92 /* Æ 00c6*/, 0x80 /* Ç 00c7*/, 
      0xfe /*   00c8*/, 0x90 /* É 00c9*/, 0xfe /*   00ca*/, 0xfe /*   00cb*/, 0xfe /*   00cc*/, 0xfe /*   00cd*/, 0xfe /*   00ce*/, 0xfe /*   00cf*/, 
      0xfe /*   00d0*/, 0xa5 /* Ñ 00d1*/, 0xfe /*   00d2*/, 0xfe /*   00d3*/, 0xfe /*   00d4*/, 0xfe /*   00d5*/, 0x99 /* Ö 00d6*/, 0xfe /*   00d7*/, 
      0xfe /*   00d8*/, 0xfe /*   00d9*/, 0xfe /*   00da*/, 0xfe /*   00db*/, 0x9a /* Ü 00dc*/, 0xfe /*   00dd*/, 0xfe /*   00de*/, 0xe1 /* ß 00df*/, 
      0x85 /* à 00e0*/, 0xa0 /* á 00e1*/, 0x83 /* â 00e2*/, 0xfe /*   00e3*/, 0x84 /* ä 00e4*/, 0x86 /* å 00e5*/, 0x91 /* æ 00e6*/, 0x87 /* ç 00e7*/, 
      0x8a /* è 00e8*/, 0x82 /* é 00e9*/, 0x88 /* ê 00ea*/, 0x89 /* ë 00eb*/, 0x8d /* ì 00ec*/, 0xa1 /* í 00ed*/, 0x8c /* î 00ee*/, 0x8b /* ï 00ef*/, 
      0xfe /*   00f0*/, 0xa4 /* ñ 00f1*/, 0x95 /* ò 00f2*/, 0xa2 /* ó 00f3*/, 0x93 /* ô 00f4*/, 0xfe /*   00f5*/, 0x94 /* ö 00f6*/, 0xf6 /* ÷ 00f7*/, 
      0xfe /*   00f8*/, 0x97 /* ù 00f9*/, 0xa3 /* ú 00fa*/, 0x96 /* û 00fb*/, 0x81 /* ü 00fc*/, 0xfe /*   00fd*/, 0xfe /*   00fe*/, 0x98 /* ÿ 00ff*/
   };

   static unsigned short box_draw[] = {
      0xc4 /* ─ 2500*/, 0xfe /*   2501*/, 0xb3 /* │ 2502*/, 0xfe /*   2503*/, 0xfe /*   2504*/, 0xfe /*   2505*/, 0xfe /*   2506*/, 0xfe /*   2507*/, 
      0xfe /*   2508*/, 0xfe /*   2509*/, 0xfe /*   250a*/, 0xfe /*   250b*/, 0xda /* ┌ 250c*/, 0xfe /*   250d*/, 0xfe /*   250e*/, 0xfe /*   250f*/, 
      0xbf /* ┐ 2510*/, 0xfe /*   2511*/, 0xfe /*   2512*/, 0xfe /*   2513*/, 0xc0 /* └ 2514*/, 0xfe /*   2515*/, 0xfe /*   2516*/, 0xfe /*   2517*/, 
      0xd9 /* ┘ 2518*/, 0xfe /*   2519*/, 0xfe /*   251a*/, 0xfe /*   251b*/, 0xc3 /* ├ 251c*/, 0xfe /*   251d*/, 0xfe /*   251e*/, 0xfe /*   251f*/, 
      0xfe /*   2520*/, 0xfe /*   2521*/, 0xfe /*   2522*/, 0xfe /*   2523*/, 0xb4 /* ┤ 2524*/, 0xfe /*   2525*/, 0xfe /*   2526*/, 0xfe /*   2527*/, 
      0xfe /*   2528*/, 0xfe /*   2529*/, 0xfe /*   252a*/, 0xfe /*   252b*/, 0xc2 /* ┬ 252c*/, 0xfe /*   252d*/, 0xfe /*   252e*/, 0xfe /*   252f*/, 
      0xfe /*   2530*/, 0xfe /*   2531*/, 0xfe /*   2532*/, 0xfe /*   2533*/, 0xc1 /* ┴ 2534*/, 0xfe /*   2535*/, 0xfe /*   2536*/, 0xfe /*   2537*/, 
      0xfe /*   2538*/, 0xfe /*   2539*/, 0xfe /*   253a*/, 0xfe /*   253b*/, 0xc5 /* ┼ 253c*/, 0xfe /*   253d*/, 0xfe /*   253e*/, 0xfe /*   253f*/, 
      0xfe /*   2540*/, 0xfe /*   2541*/, 0xfe /*   2542*/, 0xfe /*   2543*/, 0xfe /*   2544*/, 0xfe /*   2545*/, 0xfe /*   2546*/, 0xfe /*   2547*/, 
      0xfe /*   2548*/, 0xfe /*   2549*/, 0xfe /*   254a*/, 0xfe /*   254b*/, 0xfe /*   254c*/, 0xfe /*   254d*/, 0xfe /*   254e*/, 0xfe /*   254f*/, 
      0xcd /* ═ 2550*/, 0xba /* ║ 2551*/, 0xd5 /* ╒ 2552*/, 0xd6 /* ╓ 2553*/, 0xc9 /* ╔ 2554*/, 0xb8 /* ╕ 2555*/, 0xb7 /* ╖ 2556*/, 0xbb /* ╗ 2557*/, 
      0xd4 /* ╘ 2558*/, 0xd3 /* ╙ 2559*/, 0xc8 /* ╚ 255a*/, 0xbe /* ╛ 255b*/, 0xbd /* ╜ 255c*/, 0xbc /* ╝ 255d*/, 0xc6 /* ╞ 255e*/, 0xc7 /* ╟ 255f*/, 
      0xcc /* ╠ 2560*/, 0xb5 /* ╡ 2561*/, 0xb6 /* ╢ 2562*/, 0xb9 /* ╣ 2563*/, 0xd1 /* ╤ 2564*/, 0xd2 /* ╥ 2565*/, 0xcb /* ╦ 2566*/, 0xcf /* ╧ 2567*/, 
      0xd0 /* ╨ 2568*/, 0xca /* ╩ 2569*/, 0xd8 /* ╪ 256a*/, 0xd7 /* ╫ 256b*/, 0xce /* ╬ 256c*/, 0xfe /*   256d*/, 0xfe /*   256e*/, 0xfe /*   256f*/, 
      0xfe /*   2570*/, 0xfe /*   2571*/, 0xfe /*   2572*/, 0xfe /*   2573*/, 0xfe /*   2574*/, 0xfe /*   2575*/, 0xfe /*   2576*/, 0xfe /*   2577*/, 
      0xfe /*   2578*/, 0xfe /*   2579*/, 0xfe /*   257a*/, 0xfe /*   257b*/, 0xfe /*   257c*/, 0xfe /*   257d*/, 0xfe /*   257e*/, 0xfe /*   257f*/, 
      0xdf /* ▀ 2580*/, 0xfe /*   2581*/, 0xfe /*   2582*/, 0xfe /*   2583*/, 0xdc /* ▄ 2584*/, 0xfe /*   2585*/, 0xfe /*   2586*/, 0xfe /*   2587*/, 
      0xdb /* █ 2588*/, 0xfe /*   2589*/, 0xfe /*   258a*/, 0xfe /*   258b*/, 0xdd /* ▌ 258c*/, 0xfe /*   258d*/, 0xfe /*   258e*/, 0xfe /*   258f*/, 
      0xde /* ▐ 2590*/, 0xb0 /* ░ 2591*/, 0xb1 /* ▒ 2592*/, 0xb2 /* ▓ 2593*/, 0xfe /*   2594*/, 0xfe /*   2595*/, 0xfe /*   2596*/, 0xfe /*   2597*/, 
      0xfe /*   2598*/, 0xfe /*   2599*/, 0xfe /*   259a*/, 0xfe /*   259b*/, 0xfe /*   259c*/, 0xfe /*   259d*/, 0xfe /*   259e*/, 0xfe /*   259f*/, 
      0xfe /* ■ 25a0*/, 0xfe /*   25a1*/, 0xfe /*   25a2*/, 0xfe /*   25a3*/, 0xfe /*   25a4*/, 0xfe /*   25a5*/, 0xfe /*   25a6*/, 0xfe /*   25a7*/, 
      0xfe /*   25a8*/, 0xfe /*   25a9*/, 0xfe /*   25aa*/, 0xfe /*   25ab*/, 0x16 /* ▬ 25ac*/, 0xfe /*   25ad*/, 0xfe /*   25ae*/, 0xfe /*   25af*/, 
      0xfe /*   25b0*/, 0xfe /*   25b1*/, 0x1e /* ▲ 25b2*/, 0xfe /*   25b3*/, 0xfe /*   25b4*/, 0xfe /*   25b5*/, 0xfe /*   25b6*/, 0xfe /*   25b7*/, 
      0xfe /*   25b8*/, 0xfe /*   25b9*/, 0x10 /* ► 25ba*/, 0xfe /*   25bb*/, 0x1f /* ▼ 25bc*/, 0xfe /*   25bd*/, 0xfe /*   25be*/, 0xfe /*   25bf*/, 
      0xfe /*   25c0*/, 0xfe /*   25c1*/, 0xfe /*   25c2*/, 0xfe /*   25c3*/, 0x11 /* ◄ 25c4*/, 0xfe /*   25c5*/, 0xfe /*   25c6*/, 0xfe /*   25c7*/, 
      0xfe /*   25c8*/, 0xfe /*   25c9*/, 0xfe /*   25ca*/, 0x09 /* ○ 25cb*/, 0xfe /*   25cc*/, 0xfe /*   25cd*/, 0xfe /*   25ce*/, 0xfe /*   25cf*/, 
      0xfe /*   25d0*/, 0xfe /*   25d1*/, 0xfe /*   25d2*/, 0xfe /*   25d3*/, 0xfe /*   25d4*/, 0xfe /*   25d5*/, 0xfe /*   25d6*/, 0xfe /*   25d7*/, 
      0x08 /* ◘ 25d8*/, 0x0a /* ◙ 25d9*/, 0xfe /*   25da*/, 0xfe /*   25db*/, 0xfe /*   25dc*/, 0xfe /*   25dd*/, 0xfe /*   25de*/, 0xfe /*   25df*/, 
      0xfe /*   25e0*/, 0xfe /*   25e1*/, 0xfe /*   25e2*/, 0xfe /*   25e3*/, 0xfe /*   25e4*/, 0xfe /*   25e5*/, 0xfe /*   25e6*/, 0xfe /*   25e7*/, 
      0xfe /*   25e8*/, 0xfe /*   25e9*/, 0xfe /*   25ea*/, 0xfe /*   25eb*/, 0xfe /*   25ec*/, 0xfe /*   25ed*/, 0xfe /*   25ee*/, 0xfe /*   25ef*/, 
      0xfe /*   25f0*/, 0xfe /*   25f1*/, 0xfe /*   25f2*/, 0xfe /*   25f3*/, 0xfe /*   25f4*/, 0xfe /*   25f5*/, 0xfe /*   25f6*/, 0xfe /*   25f7*/, 
      0xfe /*   25f8*/, 0xfe /*   25f9*/, 0xfe /*   25fa*/, 0xfe /*   25fb*/, 0xfe /*   25fc*/, 0xfe /*   25fd*/, 0xfe /*   25fe*/, 0xfe /*   25ff*/
   };

   // Below are the Vietnamese characters that are in lower chars region of the Unicode table but cannot be mapped to anything in cp437
   // They will need to be redrawn manually later in the second font
   low_chars[0xc3] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 0); /* Ã */
   low_chars[0xe3] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 1); /* ã */
   low_chars[0xd5] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 2); /* Õ */
   low_chars[0xf5] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 3); /* õ */
   low_chars[0xca] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 16); /* Ê */
   low_chars[0xdd] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 17); /* Ý */
   low_chars[0xfd] = TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 18); /* ý */

   if (wc <= LOW_CHARS_END) {
      return low_chars[wc];
   } else if ((wc >= BOX_DRAW_BEGIN) && (wc <= BOX_DRAW_END)) {
      return box_draw[wc - BOX_DRAW_BEGIN];
   } else if ((wc >= LATIN_EXT_VIETNAMESE_BEGIN) && (wc <= LATIN_EXT_VIETNAMESE_END)) {
      return TO_VIETNAMESE_OFFSET(wc - LATIN_EXT_VIETNAMESE_BEGIN);
   /* Begin more Vietnamese fixups */
   } else if (wc == 0x0102) { // Ă
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 4);
   } else if (wc == 0x0103) { // ă
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 5);
   } else if (wc == 0x0110) { // Đ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 6);
   } else if (wc == 0x0111) { // đ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 7);
   } else if (wc == 0x0128) { // Ĩ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 8);
   } else if (wc == 0x0129) { // ĩ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 9);
   } else if (wc == 0x0168) { // Ũ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 10);
   } else if (wc == 0x0169) { // ũ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 11);
   } else if (wc == 0x01a0) { // Ơ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 12);
   } else if (wc == 0x01a1) { // ơ
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 13);
   } else if (wc == 0x01af) { // Ư
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 14);
   } else if (wc == 0x01b0) { // ư
      return TO_VIETNAMESE_OFFSET(VIETNAMESE_ADDITIONAL_MAPPING_BEGIN + 15);
   /* End more Vietnamese fixups */
   } else if (wc == 0x2302) { // ⌂
      return 0x7f;
   } else if (wc == 0x20a7) { // ₧
      return 0x9e;
   } else if (wc == 0x0192) { // ƒ
      return 0x9f;
   } else if (wc == 0x2310) { // ⌐
      return 0xa9;
   } else if (wc == 0x03b1) { // α
      return 0xe0;
   } else if (wc == 0x0393) { // Γ
      return 0xe2;
   } else if (wc == 0x03c0) { // π
      return 0xe3;
   } else if (wc == 0x03a3) { // Σ
      return 0xe4;
   } else if (wc == 0x03c3) { // σ
      return 0xe5;
   } else if (wc == 0x03c4) { // τ
      return 0xe7;
   } else if (wc == 0x03a6) { // Φ
      return 0xe8;
   } else if (wc == 0x0398) { // Θ
      return 0xe9;
   } else if (wc == 0x03a9) { // Ω
      return 0xea;
   } else if (wc == 0x03b4) { // δ
      return 0xeb;
   } else if (wc == 0x221e) { // ∞
      return 0xec;
   } else if (wc == 0x03c6) { // φ
      return 0xed;
   } else if (wc == 0x03b5) { // ε
      return 0xee;
   } else if (wc == 0x2229) { // ∩
      return 0xef;
   } else if (wc == 0x2261) { // ≡
      return 0xf0;
   } else if (wc == 0x2265) { // ≥
      return 0xf2;
   } else if (wc == 0x2264) { // ≤
      return 0xf3;
   } else if (wc == 0x2320) { // ⌠
      return 0xf4;
   } else if (wc == 0x2321) { // ⌡
      return 0xf5;
   } else if (wc == 0x2248) { // ≈
      return 0xf7;
   } else if (wc == 0x2219) { // ∙
      return 0xf9;
   } else if (wc == 0x221a) { // √
      return 0xfb;
   } else if (wc == 0x207f) { // ⁿ
      return 0xfc;
   } else {
      return 0xfe;
   }
}

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

void consume(unsigned char c, enum p_state *s, unsigned int * out) {
   if (*s == P_NORMAL) {
      if (c < 128) {
         //printf("=a=");
         *out = c;
      } else if ( (c & 0xf8) == 0xf0) {
         //printf("=3=");
         *out = (unsigned int) (c & 0x07);
         *s = P_EXPECT_THREE_CONT_B;
      } else if ( (c & 0xf0) == 0xe0) {
         //printf("=2=");
         *out = (unsigned int) (c & 0x0f);
         *s = P_EXPECT_TWO_CONT_B;
      } else if ( (c & 0xe0) == 0xc0) {
         //printf("=1=");
         *out = (unsigned int) (c & 0x1f);
         *s = P_EXPECT_ONE_CONT_B;
      } else {
         //printf("=i=");
         *s = P_INVALID;
      }
   } else if ((*s == P_EXPECT_THREE_CONT_B) || (*s == P_EXPECT_TWO_CONT_B) || (*s == P_EXPECT_ONE_CONT_B) ) {
      if ( (c & 0xc0) == 0x80 ) {
         //printf("=c=");
         *out = (*out << 6) + (c & 0x3f);
         *s = *s - 1;
      } else {
         //printf("=e=");
         *s = P_INVALID;
      }
   } else {
      //printf("=?=");
      *s = P_INVALID;
   }
}

int display_file(char * filename) {
   FILE *fptr;
   static char buff[80*25];
   static short buff_cp437e[80*25*2 + 1];
   unsigned short buff_cp437_cur;
   int cur;
   enum p_state utf8_parser_state;
   unsigned int cur_wchar;
   char schar;

   fptr = fopen(filename,"r");

   if (fptr == NULL) {
      printf("Cannot open file for reading.\n");
      return -1;
   }

   utf8_parser_state = P_NORMAL;
   while ( fgets( buff, 80*25, fptr ) != NULL ) {
      buff_cp437_cur = 0;
      for (cur = 0; cur < strlen(buff); cur++) {
         consume(buff[cur], &utf8_parser_state, &cur_wchar);
         if (utf8_parser_state == P_INVALID) {
            fclose(fptr);
            printf("Error occured while parsing the file at char %c (0x%x) position %d.\n", buff[cur], buff[cur], cur);
            return -1;
         } else if (utf8_parser_state == P_NORMAL) {
            if (cur_wchar != '\n') {
               buff_cp437e[buff_cp437_cur] = unicode_to_cp437e(cur_wchar);
               buff_cp437_cur++;
            } else if (cur_wchar == '\r') {
               // Ignore the \r
            } else {
               buff_cp437e[buff_cp437_cur] = '\r';
               buff_cp437_cur++;
               buff_cp437e[buff_cp437_cur] = '\n';
               buff_cp437_cur++;
            }
            //putch((char) unicode_to_cp437e(cur_wchar));
         }
      }
      buff_cp437e[buff_cp437_cur] = 0;
      puttexte(buff_cp437e, buff_cp437_cur);
   }
   fclose(fptr);
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


int main(int argc, char **argv) {
   if (argc != 2) {
      printf("No file name given.\n");
      return 1;
   }
   
   setup_vietnamese_font(0);
   display_file(argv[1]);
   return 0;
}
