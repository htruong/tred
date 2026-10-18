#ifndef SURFACE_H
#define SURFACE_H

// The screen as two 80x25 cell buffers: "back" is what the UI draws, "front"
// mirrors video memory. Drawing only touches back and widens a per-row dirty
// span; scr_flush writes the cells in those spans that differ from front, so
// video memory is never read and only changed cells are ever written.

#define SCR_W 80
#define SCR_H 25

typedef unsigned short cell_t; // low byte glyph, high byte attribute

// Codes > 0xff live in font block 1, selected by attribute bit 3; colours only
// use bits 0-2 so bit 3 is free to pick the font.
#define MKCELL(code, attr) ((code) > 0xff ? (cell_t) (((((attr) | 8) & 0xff) << 8) | ((code) & 0xff)) \
                                          : (cell_t) (((((attr) & 0xf7) & 0xff) << 8) | (code)))
#define CELL_CODE(c) (((c) & 0x0800) ? 0x100 | ((c) & 0xff) : (c) & 0xff)
#define CELL_ATTR(c) (((c) >> 8) & 0xf7)

void scr_init(void);
void scr_put(int x, int y, unsigned short code, unsigned char attr);
void scr_put_cells(int x, int y, const cell_t *c, int n);
void scr_fill(int x, int y, int w, int h, unsigned short code, unsigned char attr);
cell_t scr_get(int x, int y);
// Recolour a rectangle keeping its glyphs (menu and dialog shadows)
void scr_shade(int x, int y, int w, int h, unsigned char attr);
// Move a rectangle's rows by dy (-1 up, 1 down); the row it vacates is left as it was
void scr_scroll(int x, int y, int w, int h, int dy);
void scr_flush(void);
void scr_invalidate(void);
cell_t *scr_save(int x, int y, int w, int h);
void scr_restore(int x, int y, int w, int h, cell_t *saved);
// Number of video memory cells written so far (for performance checks)
extern unsigned long scr_writes;

#endif // SURFACE_H
