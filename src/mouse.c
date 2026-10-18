// DOS mouse driver interface. The text pointer only flips the colour bits of
// the cell under it, never attribute bit 3, which picks the Vietnamese font.

#include <dos.h>
#include "mouse.h"

static int present, wheel, shown;
static int lx = -1, ly = -1;
static unsigned char lbuttons;

static void call(union REGPACK *r) {
   intr(0x33, r);
}

int mouse_init(void) {
   union REGPACK r;
   void (__interrupt __far *vec)();

   vec = _dos_getvect(0x33);
   if (!vec || *(unsigned char __far *) vec == 0xcf) return 0; // no driver, or just an IRET
   r.x.ax = 0x0000;
   call(&r);
   if (r.x.ax != 0xffff) return 0;
   present = 1;
   r.x.ax = 0x0011; // CuteMouse wheel API
   call(&r);
   wheel = r.x.ax == 0x574d;
   r.x.ax = 0x000a; // software text cursor: keep glyph and font bit, invert colours
   r.x.bx = 0;
   r.x.cx = 0xffff;
   r.x.dx = 0x7700;
   call(&r);
   r.x.ax = 0x0007;
   r.x.cx = 0;
   r.x.dx = 639;
   call(&r);
   r.x.ax = 0x0008;
   r.x.cx = 0;
   r.x.dx = 199;
   call(&r);
   mouse_show();
   return 1;
}

void mouse_done(void) {
   mouse_hide();
}

void mouse_show(void) {
   union REGPACK r;

   if (!present || shown) return;
   r.x.ax = 0x0001;
   call(&r);
   shown = 1;
}

void mouse_hide(void) {
   union REGPACK r;

   if (!present || !shown) return;
   r.x.ax = 0x0002;
   call(&r);
   shown = 0;
}

int mouse_poll(mouse_ev_t *ev) {
   union REGPACK r;
   int n = 0, x, y, presses, releases, px, py, rx, ry;
   signed char w = 0;
   unsigned char b;

   if (!present) return 0;
   r.x.ax = 0x0003;
   call(&r);
   b = r.h.bl & 3;
   if (wheel) w = (signed char) r.h.bh;
   x = r.x.cx / 8;
   y = r.x.dx / 8;
   r.x.ax = 0x0005; // left presses since last time
   r.x.bx = 0;
   call(&r);
   presses = r.x.bx;
   px = r.x.cx / 8;
   py = r.x.dx / 8;
   r.x.ax = 0x0006; // left releases since last time
   r.x.bx = 0;
   call(&r);
   releases = r.x.bx;
   rx = r.x.cx / 8;
   ry = r.x.dx / 8;

   if (presses) {
      ev[n].x = px; ev[n].y = py; ev[n].buttons = 1; ev[n].pressed = 1; ev[n].released = 0; ev[n].wheel = 0;
      n++;
   }
   if (releases && (presses || (lbuttons & 1))) {
      ev[n].x = rx; ev[n].y = ry; ev[n].buttons = b; ev[n].pressed = 0; ev[n].released = 1; ev[n].wheel = 0;
      n++;
   }
   if (!n && (b & 1) && (x != lx || y != ly)) {
      ev[n].x = x; ev[n].y = y; ev[n].buttons = b; ev[n].pressed = ev[n].released = 0; ev[n].wheel = 0;
      n++; // drag
   }
   if (w) {
      ev[n].x = x; ev[n].y = y; ev[n].buttons = b; ev[n].pressed = ev[n].released = 0; ev[n].wheel = w;
      n++;
   }
   lx = x;
   ly = y;
   lbuttons = b;
   return n;
}
