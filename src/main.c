// tred (TRuong's EDitor): a Vietnamese UTF-8 text editor for DOS in the style
// of edit.com
//
// usage: tred [-k keyscript] [file]
//   -k  read keystrokes from a script instead of the keyboard (for testing);
//       when the script ends the program exits leaving the screen as it is.
//       Each scripted key is drawn before the next, as with real typing.
//   -t  with -k: write "<BIOS ticks> <cells written>" for the run to a file

#include <stdio.h>
#include <dos.h>
#include <string.h>
#include "editor.h"
#include "menu.h"
#include "surface.h"
#include "kbd.h"
#include "vgatxt.h"
#include "mouse.h"

// The BIOS timer tick count (18.2 per second)
static unsigned long bios_ticks(void) {
   return *(unsigned long __far *) MK_FP(0x40, 0x6c);
}

int main(int argc, char **argv) {
   static editor_t ed;
   const char *file = 0, *script = 0, *timing = 0;
   unsigned long start;
   FILE *tf;
   int i, k = 0, x, y;

   for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-k") && i + 1 < argc) script = argv[++i];
      else if (!strcmp(argv[i], "-t") && i + 1 < argc) timing = argv[++i];
      else file = argv[i];
   }
   if (!kbd_init(script)) {
      printf("Cannot open key script %s\n", script);
      return 1;
   }

   setup_vietnamese_font(0);
   scr_init();
   mouse_init();
   vga_cursor_shape(CURSOR_INSERT, CURSOR_END);
   ed_open(&ed, file);
   menu_bar_draw(-1);
   start = bios_ticks();

   while (!ed.quit && k != KEY_SCRIPT_END) {
      ed_render(&ed);
      scr_flush();
      ed_cursor(&ed, &x, &y);
      vga_cursor(x, y);
      // Menus and dialogs hide or change the caret; always put it back
      vga_cursor_shape(ed.ins ? CURSOR_INSERT : CURSOR_OVERWRITE, CURSOR_END);
      // Handle everything typed ahead before drawing again
      do {
         k = kbd_get();
         if (k != KEY_SCRIPT_END) ed_key(&ed, k);
         // A script that ends inside a menu or dialog leaves it on screen
         if (k != KEY_SCRIPT_END && kbd_script_ended()) return 0;
      } while (!ed.quit && k != KEY_SCRIPT_END && !kbd_scripted() && kbd_avail());
   }
   if (k == KEY_SCRIPT_END) {
      ed_render(&ed);
      scr_flush();
      ed_cursor(&ed, &x, &y);
      vga_cursor(x, y);
      if (timing && (tf = fopen(timing, "w")) != 0) {
         fprintf(tf, "%lu %lu\n", bios_ticks() - start, scr_writes);
         fclose(tf);
      }
      return 0;
   }
   mouse_done();
   restore_text_mode();
   return 0;
}
