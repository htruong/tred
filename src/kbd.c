// Keyboard input from INT 16h, or from a key script for automated testing

#include <dos.h>
#include <stdio.h>
#include <string.h>
#include "kbd.h"

static FILE *script;
static int pending = -1;
static mouse_ev_t queue[4];
static int nqueue;
mouse_ev_t mouse_ev;
static int ended;

static const struct { const char *name; int key; } names[] = {
   {"Enter", KEY_ENTER}, {"Tab", KEY_TAB}, {"BS", KEY_BS}, {"Esc", KEY_ESC}, {"Del", KEY_DEL},
   {"Ins", KEY_INS}, {"Up", KEY_UP}, {"Down", KEY_DOWN}, {"Left", KEY_LEFT}, {"Right", KEY_RIGHT},
   {"Home", KEY_HOME}, {"End", KEY_END}, {"PgUp", KEY_PGUP}, {"PgDn", KEY_PGDN},
   {"C-Left", KEY_C_LEFT}, {"C-Right", KEY_C_RIGHT}, {"C-Home", KEY_C_HOME}, {"C-End", KEY_C_END},
   {"F1", KEY_F1}, {"F9", KEY_F9}, {"F10", KEY_F10}, {"A-F", KEY_A_F}, {"A-E", KEY_A_E},
   {"A-S", KEY_A_S}, {"A-O", KEY_A_O}, {"A-H", KEY_A_H}, {"A-X", KEY_A_X},
   {"C-L", KEY_C_L}, {"C-O", KEY_C_O}, {"A-F3", KEY_A_F3}, {"C-S", KEY_C_S}, {"C-Y", KEY_C_Y}, {"C-Q", KEY_C_Q}, {"{", '{'}, {0, 0}
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
   memset(queue, 0, 2 * sizeof(queue[0]));
   if (sscanf(name, "Click:%d,%d", &queue[0].x, &queue[0].y) == 2) {
      queue[0].buttons = queue[0].pressed = 1;
      queue[1] = queue[0];
      queue[1].buttons = queue[1].pressed = 0;
      queue[1].released = 1;
      nqueue = 2;
      return KEY_MOUSE;
   }
   if (sscanf(name, "Wheel:%d", &i) == 1) {
      queue[0].wheel = (signed char) i;
      nqueue = 1;
      return KEY_MOUSE;
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

// Take the next queued mouse event as mouse_ev
static int take_mouse(void) {
   mouse_ev = queue[0];
   memmove(queue, queue + 1, --nqueue * sizeof(queue[0]));
   return KEY_MOUSE;
}

int kbd_get(void) {
   int k;

   if (nqueue && pending < 0) return take_mouse();
   if (pending >= 0) {
      k = pending;
      pending = -1;
      if (k == KEY_MOUSE) return take_mouse();
      return k;
   }
   if (script) {
      k = script_key();
      return k == KEY_MOUSE ? take_mouse() : k;
   }
   for (;;) {
      if (kbd_avail()) return kbd_get();
   }
}

int kbd_avail(void) {
   union REGPACK r;

   if (script) {
      if (nqueue) return 1;
      if (pending < 0) pending = script_key();
      return pending != KEY_SCRIPT_END;
   }
   if (pending >= 0 || nqueue) return 1;
   r.h.ah = 0x11;
   intr(0x16, &r);
   if (!(r.w.flags & INTR_ZF)) {
      pending = bios_key();
      return 1;
   }
   nqueue = mouse_poll(queue);
   return nqueue > 0;
}
