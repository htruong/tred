#ifndef EDITOR_H
#define EDITOR_H

#include "textbuf.h"
#include "ime.h"

// Screen layout, as in edit.com
#define TEXT_X 1
#define TEXT_Y 2
#define TEXT_W 78
#define TEXT_H 21

#define A_TEXT   0x17
#define A_FRAME  0x17
#define A_TITLE  0x71
#define A_SCROLL 0x70
#define A_THUMB  0x00
#define A_STATUS 0x30

enum {
   CMD_NONE, CMD_OPEN, CMD_CLOSE, CMD_SAVE, CMD_SAVEAS, CMD_EXIT, CMD_DELLINE, CMD_GOTO,
   CMD_IME_OFF, CMD_IME_TELEX, CMD_IME_VNI, CMD_TONESTYLE, CMD_WALONE, CMD_WRAP, CMD_ABOUT
};

// What one screen row of the text view shows: bytes [start, end) of a line.
// With word wrap a line takes several rows; without it, one row each.
typedef struct {
   unsigned line, start, end; // line == NO_LINE past the end of the file
} vrow_t;

#define NO_LINE 0xffffu

typedef struct {
   textbuf_t tb;
   unsigned cy, cx;      // cursor line and byte offset in it
   unsigned want;        // column within the screen row that Up/Down try to keep
   unsigned top, topoff; // first row shown: line and byte offset of its segment
   unsigned left;        // first column shown (no-wrap mode only)
   vrow_t rows[TEXT_H];  // current layout of the text view
   unsigned dlo, dhi;    // lines edited since the last render
   unsigned char relayout, redraw;
   unsigned char wrap;   // word wrap long lines
   unsigned char ins;    // insert (1) or overwrite (0)
   unsigned char quit;
   ime_t ime;
   char msg[64];         // transient status bar message
} editor_t;

int ed_open(editor_t *ed, const char *path);
void ed_key(editor_t *ed, int key);
void ed_command(editor_t *ed, int cmd);
// Draw dirty text rows and the window chrome into the surface
void ed_render(editor_t *ed);
void ed_cursor(editor_t *ed, int *x, int *y);
int ed_menu_checked(editor_t *ed, int cmd);

#endif // EDITOR_H
