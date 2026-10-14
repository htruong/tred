// vicat: a Vietnamese UTF-8 text editor for DOS in the style of edit.com
//
// usage: vicat [-k keyscript] [file]
//   -k  read keystrokes from a script instead of the keyboard (for testing);
//       when the script ends the program exits leaving the screen as it is

#include <stdio.h>
#include <string.h>
#include "editor.h"
#include "menu.h"
#include "surface.h"
#include "kbd.h"
#include "vgatxt.h"

int main(int argc, char **argv) {
   static editor_t ed;
   const char *file = 0, *script = 0;
   int i, k = 0, x, y;

   for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-k") && i + 1 < argc) script = argv[++i];
      else file = argv[i];
   }
   if (!kbd_init(script)) {
      printf("Cannot open key script %s\n", script);
      return 1;
   }

   setup_vietnamese_font(0);
   vga_cursor_shape(CURSOR_INSERT, CURSOR_END);
   scr_init();
   ed_open(&ed, file);
   menu_bar_draw(-1);

   while (!ed.quit && k != KEY_SCRIPT_END) {
      ed_render(&ed);
      scr_flush();
      ed_cursor(&ed, &x, &y);
      vga_cursor(x, y);
      // Handle everything typed ahead before drawing again
      do {
         k = kbd_get();
         if (k != KEY_SCRIPT_END) ed_key(&ed, k);
         // A script that ends inside a menu or dialog leaves it on screen
         if (k != KEY_SCRIPT_END && kbd_script_ended()) return 0;
      } while (!ed.quit && k != KEY_SCRIPT_END && kbd_avail());
   }
   if (k == KEY_SCRIPT_END) {
      ed_render(&ed);
      scr_flush();
      ed_cursor(&ed, &x, &y);
      vga_cursor(x, y);
      return 0;
   }
   restore_text_mode();
   return 0;
}
