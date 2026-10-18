#ifndef DRAW_H
#define DRAW_H

// A viewport is a rectangle of the surface; drawing in it uses view-relative
// coordinates and is clipped to it.

typedef struct {
   int x, y, w, h;
} view_t;

void v_put(const view_t *v, int c, int r, unsigned short code, unsigned char a);
void v_fill(const view_t *v, int c, int r, int w, int h, unsigned short code, unsigned char a);
// Returns the number of cells drawn
int v_text(const view_t *v, int c, int r, const char *ascii, unsigned char a);
int v_utf8(const view_t *v, int c, int r, const char *utf8, unsigned char a);
// Single-line box on the view's border, interior left alone
void v_frame(const view_t *v, unsigned char a);

#endif // DRAW_H
