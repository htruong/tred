// Keyboard input from INT 16h, or from a key script for automated testing

#include <dos.h>
#include <stdio.h>
#include <string.h>
#include "kbd.h"

static FILE *script;
static int pending = -1;
static int ended;

static const struct { const char *name; int key; } names[] = {
   {"Enter", KEY_ENTER}, {"Tab", KEY_TAB}, {"BS", KEY_BS}, {"Esc", KEY_ESC}, {"Del", KEY_DEL},
   {"Ins", KEY_INS}, {"Up", KEY_UP}, {"Down", KEY_DOWN}, {"Left", KEY_LEFT}, {"Right", KEY_RIGHT},
   {"Home", KEY_HOME}, {"End", KEY_END}, {"PgUp", KEY_PGUP}, {"PgDn", KEY_PGDN},
   {"C-Left", KEY_C_LEFT}, {"C-Right", KEY_C_RIGHT}, {"C-Home", KEY_C_HOME}, {"C-End", KEY_C_END},
   {"F1", KEY_F1}, {"F9", KEY_F9}, {"F10", KEY_F10}, {"A-F", KEY_A_F}, {"A-E", KEY_A_E},
   {"A-S", KEY_A_S}, {"A-O", KEY_A_O}, {"A-H", KEY_A_H}, {"A-X", KEY_A_X},
   {"C-O", KEY_C_O}, {"C-S", KEY_C_S}, {"C-Y", KEY_C_Y}, {"C-Q", KEY_C_Q}, {"{", '{'}, {0, 0}
};

int kbd_init(const char *path) {
   if (!path) return 1;
   script = fopen(path, "rb");
   return script != 0;
}

int kbd_scripted(void) {
   return script != 0;
}

int kbd_script_ended(void) {
   return ended;
}

static int script_key(void) {
   char name[16];
   int c, n, i;

   do {
      c = fgetc(script);
   } while (c == '\r' || c == '\n');
   if (c == EOF) {
      ended = 1;
      return KEY_SCRIPT_END;
   }
   if (c != '{') return c;
   for (n = 0; (c = fgetc(script)) != EOF && c != '}' && n < 15; n++) name[n] = (char) c;
   name[n] = 0;
   if (n == 0 && c == '}') return '{'; // "{}" is a literal brace too
   for (i = 0; names[i].name; i++) {
      if (!strcmp(names[i].name, name)) return names[i].key;
   }
   return '?';
}

static int bios_key(void) {
   union REGPACK r;

   r.h.ah = 0x10;
   intr(0x16, &r);
   if (r.h.al == 0 || (r.h.al == 0xe0 && r.h.ah != 0)) return 0x100 | r.h.ah;
   return r.h.al;
}

int kbd_get(void) {
   int k;

   if (pending >= 0) {
      k = pending;
      pending = -1;
      return k;
   }
   return script ? script_key() : bios_key();
}

int kbd_avail(void) {
   union REGPACK r;

   if (script) {
      if (pending < 0) pending = script_key();
      return pending != KEY_SCRIPT_END;
   }
   if (pending >= 0) return 1;
   r.h.ah = 0x11;
   intr(0x16, &r);
   return !(r.w.flags & INTR_ZF);
}
