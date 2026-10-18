/* Dumps VGA text memory, font plane 2 and a few registers for tools/vgarender.py.
   Build in DOS: wcl -bt=dos -ml vgadump.c */
#include <dos.h>
#include <conio.h>
#include <stdio.h>

int main(void) {
   static unsigned char buf[32768];
   FILE *f;
   unsigned char __far *p;
   unsigned i;
   unsigned char cms, pe, pal[16];

   p = MK_FP(0xB800, 0);
   for (i = 0; i < 4000; i++) buf[i] = p[i];
   f = fopen("TEXT.BIN", "wb");
   fwrite(buf, 1, 4000, f);
   fclose(f);

   outp(0x3C4, 3);
   cms = inp(0x3C5);
   inp(0x3DA);
   outp(0x3C0, 0x12);
   pe = inp(0x3C1);
   for (i = 0; i < 16; i++) {
      inp(0x3DA);
      outp(0x3C0, i);
      pal[i] = inp(0x3C1);
   }
   inp(0x3DA);
   outp(0x3C0, 0x20);

   /* Map plane 2 (the fonts) at A000, read it, then restore text mode mapping */
   _disable();
   outpw(0x3C4, 0x0402); outpw(0x3C4, 0x0704);
   outpw(0x3CE, 0x0204); outpw(0x3CE, 0x0005); outpw(0x3CE, 0x0406);
   p = MK_FP(0xA000, 0);
   for (i = 0; i < 32768; i++) buf[i] = p[i];
   outpw(0x3C4, 0x0302); outpw(0x3C4, 0x0304);
   outpw(0x3CE, 0x0004); outpw(0x3CE, 0x1005); outpw(0x3CE, 0x0E06);
   _enable();
   f = fopen("FONT.BIN", "wb");
   fwrite(buf, 1, 32768, f);
   fclose(f);

   f = fopen("REGS.TXT", "w");
   outp(0x3D4, 9);
   fprintf(f, "%d %d %d", cms, pe, (inp(0x3D5) & 0x1f) + 1);
   for (i = 0; i < 16; i++) fprintf(f, " %d", pal[i]);
   outp(0x3D4, 0x0a);
   fprintf(f, " %d", inp(0x3D5));
   outp(0x3D4, 0x0b);
   fprintf(f, " %d", inp(0x3D5));
   outp(0x3D4, 0x0e);
   fprintf(f, " %d", inp(0x3D5) << 8);
   outp(0x3D4, 0x0f);
   fprintf(f, " %d\n", inp(0x3D5));
   fclose(f);
   return 0;
}
