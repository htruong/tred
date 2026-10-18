#ifndef KBD_H
#define KBD_H

#include "mouse.h"

// Keys are the ASCII code, or 0x100 | scan code for keys without one
#define KEY_ENTER  0x0d
#define KEY_BS     0x08
#define KEY_TAB    0x09
#define KEY_ESC    0x1b
#define KEY_UP     0x148
#define KEY_DOWN   0x150
#define KEY_LEFT   0x14b
#define KEY_RIGHT  0x14d
#define KEY_HOME   0x147
#define KEY_END    0x14f
#define KEY_PGUP   0x149
#define KEY_PGDN   0x151
#define KEY_INS    0x152
#define KEY_DEL    0x153
#define KEY_C_LEFT  0x173
#define KEY_C_RIGHT 0x174
#define KEY_C_HOME  0x177
#define KEY_C_END   0x175
#define KEY_C_PGUP  0x184
#define KEY_C_PGDN  0x176
#define KEY_F1     0x13b
#define KEY_F9     0x143
#define KEY_F10    0x144
#define KEY_A_F    0x121
#define KEY_A_E    0x112
#define KEY_A_S    0x11f
#define KEY_A_O    0x118
#define KEY_A_H    0x123
#define KEY_A_X    0x12d
#define KEY_C_O    0x0f
#define KEY_C_S    0x13
#define KEY_C_Y    0x19
#define KEY_C_Q    0x11
// Returned once a key script has run out
#define KEY_SCRIPT_END 0x7fff
// A mouse event; details in mouse_ev
#define KEY_MOUSE  0x7ffe
#define KEY_A_F3   0x16a

// script: NULL reads the BIOS keyboard, else keys are read from that file:
// literal characters and {Name} tokens such as {Enter} {Up} {C-S} {F9} {{};
// raw newlines in the file are ignored.
// Scripts can also click with {Click:x,y} and scroll with {Wheel:n}.
int kbd_init(const char *script);
extern mouse_ev_t mouse_ev;
int kbd_get(void);
int kbd_avail(void);
int kbd_scripted(void);
// The key script ran out (possibly inside a menu or dialog)
int kbd_script_ended(void);

#endif // KBD_H
