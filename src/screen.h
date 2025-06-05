#ifndef SCREEN_H
#define SCREEN_H

#include <dos.h>

void puttext(unsigned char __far *s, unsigned char textattr, unsigned short len);
void puttexte(unsigned short __far *s, unsigned short len);

enum display_fontslot {
   SLOT_UNKNOWN,
   SLOT_NORMAL,
   SLOT_EXTENDED
};

#endif // SCREEN_H