#ifndef SCREEN_H
#define SCREEN_H

void puttext(unsigned char __far *s, unsigned char textattr, unsigned short len);
void puttexte(unsigned short __far *s, unsigned short len);

enum display_fontslot {
   SLOT_UNKNOWN,
   SLOT_NORMAL,
   SLOT_EXTENDED
};

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#endif // SCREEN_H
