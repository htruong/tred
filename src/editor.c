// The edit window: cursor movement, editing, Vietnamese input and drawing
// the text view plus edit.com's frame, scrollbars and status bar.
//
// The text view is a table of screen rows (a whole line, or with word wrap
// one wrapped piece of a line). It is laid out again only after an edit or a
// scroll, and only rows whose content changed are decoded and redrawn; the
// chrome is cheap and redrawn every time, and the surface writes only what
// changed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "port.h"
#include "editor.h"
#include "menu.h"
#include "surface.h"
#include "draw.h"
#include "utf8.h"
#include "vncvt.h"
#include "vgatxt.h"
#include "kbd.h"

#define TAB_W 8
#define MAX_COL 255
#define WRAP_W (TEXT_W - 1) // leave a column for the cursor after a full row

static line_t *cur_line(editor_t *ed) {
   return &ed->tb.ln[ed->cy];
}

static unsigned next_col(unsigned col, unsigned cp) {
   return cp == '\t' ? (col / TAB_W + 1) * TAB_W : col + 1;
}

// Display column of byte off, counting from byte start of the line
static unsigned col_in(const line_t *ln, unsigned start, unsigned off) {
   unsigned p = start, col = 0, cp;

   while (p < off && p < ln->len) {
      p += utf8_dec(ln->p + p, ln->p + ln->len, &cp);
      col = next_col(col, cp);
   }
   return col;
}

// Byte offset at column col of the screen row [start, end); on a row that
// wraps, stop before its end so the cursor stays on that row
static unsigned off_at(const line_t *ln, unsigned start, unsigned end, unsigned col) {
   unsigned p = start, c = 0, n, nc, cp;

   while (p < end && c < col) {
      n = utf8_dec(ln->p + p, ln->p + ln->len, &cp);
      nc = next_col(c, cp);
      if (nc > col) break;
      c = nc;
      p += n;
   }
   if (end < ln->len && p >= end && p > start) p = utf8_prev(ln->p, end);
   return p;
}

// End of the screen row starting at byte start: where the next row starts,
// or the line length for its last row. Word wrap breaks after the last space
// that fits, or inside a word longer than the row.
static unsigned seg_end(const editor_t *ed, const line_t *ln, unsigned start) {
   unsigned p = start, col = 0, brk = 0, n, cp;

   if (!ed->wrap) return ln->len;
   while (p < ln->len) {
      n = utf8_dec(ln->p + p, ln->p + ln->len, &cp);
      if (cp == ' ') {
         brk = p + n; // spaces may hang past the edge
      } else if (next_col(col, cp) > WRAP_W) {
         return brk > start ? brk : p;
      }
      col = next_col(col, cp);
      p += n;
   }
   return ln->len;
}

// Start (and end) of the screen row of line ln that holds byte off
static unsigned seg_start_of(const editor_t *ed, const line_t *ln, unsigned off, unsigned *end) {
   unsigned s = 0, e;

   for (;;) {
      e = seg_end(ed, ln, s);
      if (off < e || e >= ln->len) break;
      s = e;
   }
   if (end) *end = e;
   return s;
}

static int row_up(editor_t *ed, unsigned *l, unsigned *s) {
   line_t *ln;
   unsigned p = 0, e;

   if (*s == 0) {
      if (*l == 0) return 0;
      (*l)--;
      *s = ed->tb.ln[*l].len + 1; // find the last row of the line above
   }
   ln = &ed->tb.ln[*l];
   for (;;) {
      e = seg_end(ed, ln, p);
      if (e >= *s || e >= ln->len) break;
      p = e;
   }
   *s = p;
   return 1;
}

static int row_down(editor_t *ed, unsigned *l, unsigned *s) {
   line_t *ln = &ed->tb.ln[*l];
   unsigned e = seg_end(ed, ln, *s);

   if (e < ln->len) {
      *s = e;
      return 1;
   }
   if (*l + 1 >= ed->tb.n) return 0;
   (*l)++;
   *s = 0;
   return 1;
}

static void mark_line(editor_t *ed, unsigned l) {
   if (l < ed->dlo) ed->dlo = l;
   if (l > ed->dhi) ed->dhi = l;
   ed->relayout = 1;
}

// Lines from l on moved (a line was split, joined or deleted)
static void mark_from(editor_t *ed, unsigned l) {
   if (l < ed->dlo) ed->dlo = l;
   ed->dhi = NO_LINE - 1;
   ed->relayout = 1;
}

static void set_want(editor_t *ed) {
   line_t *ln = cur_line(ed);

   ed->want = col_in(ln, seg_start_of(ed, ln, ed->cx, 0), ed->cx);
}

static void report(editor_t *ed, int err) {
   if (err == TB_ENOMEM) strcpy(ed->msg, "Out of memory");
   else if (err == TB_ETOOBIG) strcpy(ed->msg, "Line or file too long");
   else if (err == TB_EIO) strcpy(ed->msg, "Disk error");
}

int ed_open(editor_t *ed, const char *path) {
   int err = TB_OK;
   FILE *f;

   memset(ed, 0, sizeof(*ed));
   tb_init(&ed->tb);
   ime_init(&ed->ime);
   ed->ins = 1;
   ed->wrap = 1;
   ed->dlo = NO_LINE;
   ed->redraw = 1;
   if (path) {
      strncpy(ed->tb.name, path, sizeof(ed->tb.name) - 1);
      f = fopen(path, "rb");
      if (f) {
         fclose(f);
         err = tb_load(&ed->tb, path);
         strncpy(ed->tb.name, path, sizeof(ed->tb.name) - 1);
         report(ed, err);
      }
   }
   return err;
}

// ---- drawing ----

static void render_row(editor_t *ed, int r) {
   cell_t row[TEXT_W];
   const vrow_t *vr = &ed->rows[r];
   unsigned col = 0, end = ed->left + TEXT_W, cp;
   const char *p, *e;
   unsigned short code;
   int i;

   for (i = 0; i < TEXT_W; i++) row[i] = MKCELL(' ', A_TEXT);
   if (vr->line != NO_LINE) {
      p = ed->tb.ln[vr->line].p + vr->start;
      e = ed->tb.ln[vr->line].p + vr->end;
      while (p < e && col < end) {
         p += utf8_dec(p, e, &cp);
         if (cp != '\t' && col >= ed->left) {
            code = cp == UTF8_BAD ? 0xfe : unicode_to_cp437e(cp);
            row[col - ed->left] = MKCELL(code, A_TEXT);
         }
         col = next_col(col, cp);
      }
   }
   scr_put_cells(TEXT_X, TEXT_Y + r, row, TEXT_W);
}

// Lay the text out from the top row and redraw the rows whose content changed
static void layout(editor_t *ed) {
   unsigned l, s, r;
   vrow_t nr;
   line_t *ln;

   if (ed->top >= ed->tb.n) {
      ed->top = ed->tb.n - 1;
      ed->topoff = 0;
   }
   ed->topoff = seg_start_of(ed, &ed->tb.ln[ed->top], ed->topoff, 0);
   l = ed->top;
   s = ed->topoff;
   for (r = 0; r < TEXT_H; r++) {
      nr.line = l;
      nr.start = nr.end = 0;
      if (l != NO_LINE) {
         ln = &ed->tb.ln[l];
         nr.start = s;
         nr.end = seg_end(ed, ln, s);
         if (nr.end < ln->len) s = nr.end;
         else if (l + 1 < ed->tb.n) { l++; s = 0; }
         else l = NO_LINE;
      }
      if (ed->redraw || nr.line != ed->rows[r].line || nr.start != ed->rows[r].start || nr.end != ed->rows[r].end ||
          (nr.line != NO_LINE && nr.line >= ed->dlo && nr.line <= ed->dhi)) {
         ed->rows[r] = nr;
         render_row(ed, r);
      }
   }
   ed->dlo = NO_LINE;
   ed->dhi = 0;
   ed->relayout = ed->redraw = 0;
}

static int cursor_row(editor_t *ed) {
   unsigned cs = seg_start_of(ed, cur_line(ed), ed->cx, 0);
   int r;

   for (r = 0; r < TEXT_H; r++) {
      if (ed->rows[r].line == ed->cy && ed->rows[r].start == cs) return r;
   }
   return -1;
}

static void scroll_to_cursor(editor_t *ed) {
   unsigned cs = seg_start_of(ed, cur_line(ed), ed->cx, 0), l = ed->cy, s = cs, col;
   int i;

   if (!ed->wrap) {
      // Jump horizontally in steps like edit.com does instead of one column at a time
      col = col_in(cur_line(ed), 0, ed->cx);
      if (col < ed->left) {
         ed->left = col < 20 ? 0 : col - 20;
         ed->redraw = 1;
      } else if (col >= ed->left + TEXT_W) {
         ed->left = col - TEXT_W + 20;
         ed->redraw = 1;
      }
   }
   if (cursor_row(ed) >= 0) return;
   if (ed->cy > ed->top || (ed->cy == ed->top && cs > ed->topoff)) {
      // Below the window: make the cursor row the bottom row
      for (i = 1; i < TEXT_H && row_up(ed, &l, &s); i++);
   }
   ed->top = l;
   ed->topoff = s;
   ed->relayout = 1;
}

static void render_chrome(editor_t *ed) {
   static const view_t win = {0, 1, 80, 23}, stat = {0, 24, 80, 1};
   char title[40], pos[16];
   const char *name = ed->tb.name[0] ? ed->tb.name : "Untitled";
   const char *slash;
   int i, tw, thumb, nl = ed->tb.n;

   // Window frame with the file name centred in the top edge
   v_put(&win, 0, 0, 0xda, A_FRAME);
   v_fill(&win, 1, 0, 78, 1, 0xc4, A_FRAME);
   v_put(&win, 79, 0, 0xbf, A_FRAME);
   // Close box for the file, as in Turbo Pascal windows
   v_text(&win, 2, 0, "[ ]", A_FRAME);
   v_put(&win, 3, 0, 0xfe, (A_FRAME & 0xf0) | COLOR_WHITE);
   if ((slash = strrchr(name, '\\')) != 0) name = slash + 1;
   sprintf(title, " %.30s ", name);
   for (i = 0; title[i]; i++) title[i] = (char) toupper((unsigned char) title[i]);
   tw = (int) strlen(title);
   v_text(&win, (80 - tw) / 2, 0, title, A_TITLE);
   v_fill(&win, 0, 1, 1, TEXT_H, 0xb3, A_FRAME);

   // Vertical scrollbar
   v_put(&win, 79, 1, 0x18, A_SCROLL);
   v_fill(&win, 79, 2, 1, TEXT_H - 2, 0xb0, A_SCROLL);
   v_put(&win, 79, TEXT_H, 0x19, A_SCROLL);
   thumb = nl > 1 ? (int) ((long) ed->cy * (TEXT_H - 3) / (nl - 1)) : 0;
   v_put(&win, 79, 2 + thumb, ' ', A_THUMB);

   // Horizontal scrollbar on the bottom edge
   v_put(&win, 0, 22, 0xb3, A_FRAME);
   v_put(&win, 1, 22, 0x1b, A_SCROLL);
   v_fill(&win, 2, 22, 76, 1, 0xb0, A_SCROLL);
   v_put(&win, 78, 22, 0x1a, A_SCROLL);
   v_put(&win, 79, 22, 0xb3, A_FRAME);
   thumb = ed->wrap ? 0 : (int) ((long) ed->left * 75 / (MAX_COL - TEXT_W));
   v_put(&win, 2 + (thumb > 75 ? 75 : thumb), 22, ' ', A_THUMB);

   // Status bar
   v_fill(&stat, 0, 0, 80, 1, ' ', A_STATUS);
   if (ed->msg[0]) {
      v_utf8(&stat, 1, 0, ed->msg, A_STATUS);
   } else {
      v_text(&stat, 1, 0, "tred   <F10=Menu>  <F9=Input method>", A_STATUS);
      v_text(&stat, 8, 0, "<F10=Menu>", (A_STATUS & 0xf0) | COLOR_WHITE);
      v_text(&stat, 20, 0, "<F9=Input method>", (A_STATUS & 0xf0) | COLOR_WHITE);
   }
   v_put(&stat, 62, 0, 0xb3, A_STATUS);
   v_text(&stat, 64, 0, ime_name(ed->ime.mode), A_STATUS);
   if (!ed->ins) v_text(&stat, 59, 0, "OVR", A_STATUS);
   sprintf(pos, "%05u:%03u", ed->cy + 1, col_in(cur_line(ed), 0, ed->cx) + 1);
   v_text(&stat, 70, 0, pos, A_STATUS);
}

void ed_render(editor_t *ed) {
   if (ed->relayout || ed->redraw) layout(ed);
   scroll_to_cursor(ed);
   if (ed->relayout || ed->redraw) layout(ed);
   render_chrome(ed);
}

void ed_cursor(editor_t *ed, int *x, int *y) {
   line_t *ln = cur_line(ed);
   int r = cursor_row(ed), c;

   c = (int) col_in(ln, seg_start_of(ed, ln, ed->cx, 0), ed->cx) - (int) ed->left;
   *x = TEXT_X + (c < TEXT_W ? c : TEXT_W - 1);
   *y = TEXT_Y + (r < 0 ? 0 : r);
}

// ---- editing ----

static void insert_bytes(editor_t *ed, const char *s, unsigned n) {
   int err = tb_insert(&ed->tb, ed->cy, ed->cx, s, n);

   if (err != TB_OK) {
      report(ed, err);
      return;
   }
   ed->cx += n;
   mark_line(ed, ed->cy);
}

// The word (letters the IME may rewrite) ending at the cursor
static int word_left(editor_t *ed, unsigned short *w, unsigned *start) {
   line_t *ln = cur_line(ed);
   unsigned short tmp[IME_MAXWORD + 1];
   unsigned off = ed->cx, prev, cp;
   int n = 0, i;

   while (off > 0 && n <= IME_MAXWORD) {
      prev = utf8_prev(ln->p, off);
      utf8_dec(ln->p + prev, ln->p + off, &cp);
      if (!ime_word_char(&ed->ime, cp)) break;
      tmp[n++] = (unsigned short) cp;
      off = prev;
   }
   for (i = 0; i < n; i++) w[i] = tmp[n - 1 - i];
   *start = off;
   return n;
}

static void type_key(editor_t *ed, int key) {
   unsigned short word[IME_MAXWORD + 1], out[IME_MAXWORD + 2];
   char buf[3 * (IME_MAXWORD + 2)], ch;
   unsigned start, off, cp, len = 0;
   int n, nout, k, i, err;
   line_t *ln = cur_line(ed);

   n = word_left(ed, word, &start);
   if (ime_key(&ed->ime, word, n, key, out, &nout)) {
      // Replace only the part of the word that changed
      for (k = 0; k < n && k < nout && word[k] == out[k]; k++);
      for (off = start, i = 0; i < k; i++) off += utf8_dec(ln->p + off, ln->p + ln->len, &cp);
      for (i = k; i < nout; i++) len += utf8_enc(out[i], buf + len);
      tb_delete(&ed->tb, ed->cy, off, ed->cx - off);
      ed->cx = off;
      insert_bytes(ed, buf, len);
   } else {
      if (!ime_word_char(&ed->ime, (unsigned) key)) ime_reset(&ed->ime);
      if (!ed->ins && ed->cx < ln->len) {
         err = (int) utf8_dec(ln->p + ed->cx, ln->p + ln->len, &cp);
         tb_delete(&ed->tb, ed->cy, ed->cx, (unsigned) err);
      }
      ch = (char) key;
      insert_bytes(ed, &ch, 1);
   }
   set_want(ed);
}

static void newline(editor_t *ed) {
   line_t *ln = cur_line(ed);
   unsigned indent = 0;
   int err;

   // Keep the indentation of the current line, like edit.com
   while (indent < ln->len && indent < ed->cx && (ln->p[indent] == ' ' || ln->p[indent] == '\t')) indent++;
   if ((err = tb_split(&ed->tb, ed->cy, ed->cx)) != TB_OK) {
      report(ed, err);
      return;
   }
   mark_from(ed, ed->cy);
   ed->cy++;
   ed->cx = 0;
   if (indent) {
      ln = &ed->tb.ln[ed->cy - 1];
      if (tb_insert(&ed->tb, ed->cy, 0, ln->p, indent) == TB_OK) ed->cx = indent;
   }
   set_want(ed);
}

static void backspace(editor_t *ed) {
   unsigned prev;
   int err;

   if (ed->cx > 0) {
      prev = utf8_prev(cur_line(ed)->p, ed->cx);
      tb_delete(&ed->tb, ed->cy, prev, ed->cx - prev);
      ed->cx = prev;
      mark_line(ed, ed->cy);
   } else if (ed->cy > 0) {
      ed->cx = ed->tb.ln[ed->cy - 1].len;
      if ((err = tb_join(&ed->tb, ed->cy - 1)) != TB_OK) {
         ed->cx = 0;
         report(ed, err);
         return;
      }
      ed->cy--;
      mark_from(ed, ed->cy);
   }
   set_want(ed);
}

static void delete_char(editor_t *ed) {
   line_t *ln = cur_line(ed);
   unsigned cp;
   int err;

   if (ed->cx < ln->len) {
      tb_delete(&ed->tb, ed->cy, ed->cx, utf8_dec(ln->p + ed->cx, ln->p + ln->len, &cp));
      mark_line(ed, ed->cy);
   } else if (ed->cy + 1 < ed->tb.n) {
      if ((err = tb_join(&ed->tb, ed->cy)) != TB_OK) report(ed, err);
      mark_from(ed, ed->cy);
   }
}

static int is_word(const line_t *ln, unsigned off) {
   unsigned char c = (unsigned char) ln->p[off];

   return isalnum(c) || c >= 0x80 || c == '_';
}

static void word_right(editor_t *ed) {
   line_t *ln = cur_line(ed);

   if (ed->cx >= ln->len) {
      if (ed->cy + 1 < ed->tb.n) {
         ed->cy++;
         ed->cx = 0;
      }
      return;
   }
   while (ed->cx < ln->len && is_word(ln, ed->cx)) ed->cx++;
   while (ed->cx < ln->len && !is_word(ln, ed->cx)) ed->cx++;
}

static void word_left_move(editor_t *ed) {
   line_t *ln = cur_line(ed);

   if (ed->cx == 0) {
      if (ed->cy > 0) {
         ed->cy--;
         ed->cx = cur_line(ed)->len;
      }
      return;
   }
   while (ed->cx > 0 && !is_word(ln, ed->cx - 1)) ed->cx--;
   while (ed->cx > 0 && is_word(ln, ed->cx - 1)) ed->cx--;
}

// Move the cursor d screen rows, keeping its column
static void move_vert(editor_t *ed, int d) {
   line_t *ln = cur_line(ed);
   unsigned l = ed->cy, st = seg_start_of(ed, ln, ed->cx, 0), e;

   for (; d < 0 && row_up(ed, &l, &st); d++);
   for (; d > 0 && row_down(ed, &l, &st); d--);
   ed->cy = l;
   ln = cur_line(ed);
   e = seg_end(ed, ln, st);
   ed->cx = off_at(ln, st, e, ed->want);
}

static void page(editor_t *ed, int dir) {
   unsigned l = ed->top, s = ed->topoff;
   int i;

   for (i = 0; i < TEXT_H - 1; i++) {
      if (!(dir < 0 ? row_up(ed, &l, &s) : row_down(ed, &l, &s))) break;
   }
   ed->top = l;
   ed->topoff = s;
   ed->relayout = 1;
   move_vert(ed, dir * (TEXT_H - 1));
}

static void save(editor_t *ed, int ask) {
   char name[80];
   int err;

   strcpy(name, ed->tb.name);
   if (ask || !name[0]) {
      if (!inputbox("Save As", "File Name:", name, 64) || !name[0]) return;
   }
   if ((err = tb_save(&ed->tb, name)) != TB_OK) {
      report(ed, err);
      return;
   }
   strcpy(ed->tb.name, name);
   sprintf(ed->msg, "Saved %.40s", name);
}

// Before the text is thrown away: offer to save it. Returns 0 to cancel.
static int confirm_discard(editor_t *ed) {
   static const char * const lines[] = {"Loaded file is not saved. Save it now?", 0};
   static const char * const buttons[] = {"Yes", "No", "Cancel"};
   int b;

   if (!ed->tb.modified) return 1;
   b = msgbox(0, lines, buttons, 3, 0);
   if (b < 0 || b == 2) return 0;
   if (b == 0) {
      save(ed, 0);
      if (ed->tb.modified) return 0;
   }
   return 1;
}

static void reset_view(editor_t *ed);

static void quit(editor_t *ed) {
   if (confirm_discard(ed)) ed->quit = 1;
}

static void open_file(editor_t *ed) {
   char name[80];
   textbuf_t tb;
   FILE *f;
   int err;

   if (!confirm_discard(ed)) return;
   name[0] = 0;
   if (!open_dialog(name, 64)) return;
   f = fopen(name, "rb");
   if (!f) {
      sprintf(ed->msg, "File not found: %.40s", name);
      return;
   }
   fclose(f);
   // Load into a new buffer so a failure leaves the current text alone
   tb_init(&tb);
   if ((err = tb_load(&tb, name)) != TB_OK) {
      tb_free(&tb);
      report(ed, err);
      return;
   }
   tb_free(&ed->tb);
   ed->tb = tb;
   strncpy(ed->tb.name, name, sizeof(ed->tb.name) - 1);
   reset_view(ed);
}

static void go_to_line(editor_t *ed) {
   char buf[8];
   long l;

   buf[0] = 0;
   if (!inputbox("Go To Line", "Line number:", buf, 5)) return;
   l = atol(buf);
   if (l < 1) return;
   ed->cy = (unsigned) (l > (long) ed->tb.n ? ed->tb.n - 1 : l - 1);
   ed->cx = 0;
   set_want(ed);
}

static void reset_view(editor_t *ed) {
   ed->cy = ed->cx = ed->want = 0;
   ed->top = ed->topoff = ed->left = 0;
   ed->redraw = 1;
}

// Close the file, leaving an empty Untitled one
static void close_file(editor_t *ed) {
   if (!confirm_discard(ed)) return;
   tb_free(&ed->tb);
   tb_init(&ed->tb);
   reset_view(ed);
}

// Scroll the view by d rows, taking the cursor along when it would go off screen
static void scroll_view(editor_t *ed, int d) {
   unsigned l = ed->top, s = ed->topoff;
   const vrow_t *vr;
   int r;

   for (; d < 0 && row_up(ed, &l, &s); d++);
   for (; d > 0 && row_down(ed, &l, &s); d--);
   ed->top = l;
   ed->topoff = s;
   layout(ed);
   if (cursor_row(ed) >= 0) return;
   // The cursor scrolled off: put it on the first or last row shown
   for (r = TEXT_H - 1; r > 0 && ed->rows[r].line == NO_LINE; r--);
   vr = (ed->cy < l || (ed->cy == l && ed->cx < s)) ? &ed->rows[0] : &ed->rows[r];
   ed->cy = vr->line;
   ed->cx = off_at(cur_line(ed), vr->start, vr->end, ed->want);
}

static void click_text(editor_t *ed, int x, int y) {
   const vrow_t *vr = &ed->rows[y - TEXT_Y];

   if (vr->line == NO_LINE) {
      ed->cy = ed->tb.n - 1;
      ed->cx = cur_line(ed)->len;
   } else {
      ed->cy = vr->line;
      ed->cx = off_at(cur_line(ed), vr->start, vr->end, x - TEXT_X + ed->left);
   }
   set_want(ed);
}

static void ed_mouse(editor_t *ed) {
   const mouse_ev_t *m = &mouse_ev;
   int x = m->x, y = m->y, thumb, menu, col;
   line_t *ln;

   if (m->wheel) {
      scroll_view(ed, m->wheel * 3);
      return;
   }
   // Placing the cursor follows drags; everything else acts on the press
   if ((m->pressed || (m->buttons & 1)) && x >= TEXT_X && x < TEXT_X + TEXT_W && y >= TEXT_Y && y < TEXT_Y + TEXT_H) {
      click_text(ed, x, y);
      return;
   }
   if (!m->pressed) return;
   if (y == 0) {
      if ((menu = menu_hit(x)) >= 0) ed_command(ed, menu_run(ed, menu));
   } else if (y == 1 && x >= 2 && x <= 4) {
      ed_command(ed, CMD_CLOSE);
   } else if (x == 79 && y >= TEXT_Y && y < TEXT_Y + TEXT_H) {
      thumb = TEXT_Y + 1 + (ed->tb.n > 1 ? (int) ((long) ed->cy * (TEXT_H - 3) / (ed->tb.n - 1)) : 0);
      if (y == TEXT_Y) scroll_view(ed, -1);
      else if (y == TEXT_Y + TEXT_H - 1) scroll_view(ed, 1);
      else if (y < thumb) page(ed, -1);
      else if (y > thumb) page(ed, 1);
   } else if (y == TEXT_Y + TEXT_H && x >= 1 && x <= 78 && !ed->wrap) {
      // Horizontal scrollbar: arrows move a column, the track a screenful
      ln = cur_line(ed);
      col = (int) col_in(ln, 0, ed->cx) + (x == 1 ? -1 : x == 78 ? 1 : x < 40 ? -(TEXT_W - 20) : TEXT_W - 20);
      if (col < 0) col = 0;
      ed->cx = off_at(ln, 0, ln->len, (unsigned) col);
      set_want(ed);
   } else if (y == 24) {
      if (x >= 8 && x <= 17) ed_command(ed, menu_run(ed, 0));
      else if ((x >= 20 && x <= 36) || (x >= 64 && x <= 69)) ed->ime.mode = (unsigned char) ((ed->ime.mode + 1) % IME_MODES);
      else if (x >= 59 && x <= 61) ed->ins = !ed->ins;
   }
}

int ed_menu_checked(editor_t *ed, int cmd) {
   switch (cmd) {
   case CMD_IME_OFF: return ed->ime.mode == IME_OFF;
   case CMD_IME_TELEX: return ed->ime.mode == IME_TELEX;
   case CMD_IME_VNI: return ed->ime.mode == IME_VNI;
   case CMD_TONESTYLE: return ed->ime.newstyle;
   case CMD_WRAP: return ed->wrap;
   case CMD_WALONE: return ed->ime.w_alone;
   }
   return 0;
}

void ed_command(editor_t *ed, int cmd) {
   static const char * const about[] = {
      "tred - TRuong's EDitor", "", "A Vietnamese text editor for DOS",
      "Telex / VNI input like Unikey, UTF-8 files", "Font after BKED, look after edit.com", 0
   };
   static const char * const ok[] = {"OK"};

   // Dialogs draw over the editor, so bring it up to date first
   ed_render(ed);
   switch (cmd) {
   case CMD_OPEN: open_file(ed); break;
   case CMD_CLOSE: close_file(ed); break;
   case CMD_SAVE: save(ed, 0); break;
   case CMD_SAVEAS: save(ed, 1); break;
   case CMD_EXIT: quit(ed); break;
   case CMD_GOTO: go_to_line(ed); break;
   case CMD_DELLINE:
      tb_delete_line(&ed->tb, ed->cy);
      if (ed->cy >= ed->tb.n) ed->cy = ed->tb.n - 1;
      ed->cx = 0;
      set_want(ed);
      mark_from(ed, ed->cy);
      break;
   case CMD_IME_OFF: ed->ime.mode = IME_OFF; break;
   case CMD_IME_TELEX: ed->ime.mode = IME_TELEX; break;
   case CMD_IME_VNI: ed->ime.mode = IME_VNI; break;
   case CMD_TONESTYLE: ed->ime.newstyle = !ed->ime.newstyle; break;
   case CMD_WALONE: ed->ime.w_alone = !ed->ime.w_alone; break;
   case CMD_WRAP:
      ed->wrap = !ed->wrap;
      ed->left = ed->topoff = 0;
      ed->redraw = 1;
      set_want(ed);
      break;
   case CMD_ABOUT: msgbox("About", about, ok, 1, 0); break;
   }
   ime_reset(&ed->ime);
}

void ed_key(editor_t *ed, int key) {
   line_t *ln = cur_line(ed);
   unsigned cp, spaces, end;
   static const char blanks[] = "        ";

   ed->msg[0] = 0;
   switch (key) {
   case KEY_LEFT:
      if (ed->cx > 0) ed->cx = utf8_prev(ln->p, ed->cx);
      else if (ed->cy > 0) ed->cx = ed->tb.ln[--ed->cy].len;
      set_want(ed);
      break;
   case KEY_RIGHT:
      if (ed->cx < ln->len) ed->cx += utf8_dec(ln->p + ed->cx, ln->p + ln->len, &cp);
      else if (ed->cy + 1 < ed->tb.n) { ed->cy++; ed->cx = 0; }
      set_want(ed);
      break;
   case KEY_UP: move_vert(ed, -1); break;
   case KEY_DOWN: move_vert(ed, 1); break;
   case KEY_HOME: // start and end of the screen row
      ed->cx = seg_start_of(ed, ln, ed->cx, &end);
      set_want(ed);
      break;
   case KEY_END:
      seg_start_of(ed, ln, ed->cx, &end);
      ed->cx = end < ln->len ? utf8_prev(ln->p, end) : end;
      set_want(ed);
      break;
   case KEY_PGUP: page(ed, -1); break;
   case KEY_PGDN: page(ed, 1); break;
   case KEY_C_HOME: ed->cy = 0; ed->cx = 0; set_want(ed); break;
   case KEY_C_END: ed->cy = ed->tb.n - 1; ed->cx = cur_line(ed)->len; set_want(ed); break;
   case KEY_C_LEFT: word_left_move(ed); set_want(ed); break;
   case KEY_C_RIGHT: word_right(ed); set_want(ed); break;
   case KEY_INS:
      ed->ins = !ed->ins;
      vga_cursor_shape(ed->ins ? CURSOR_INSERT : CURSOR_OVERWRITE, CURSOR_END);
      break;
   case KEY_ENTER: newline(ed); break;
   case KEY_BS: backspace(ed); break;
   case KEY_DEL: delete_char(ed); break;
   case KEY_TAB:
      // Spaces up to the next tab stop, as edit.com does
      spaces = TAB_W - col_in(ln, 0, ed->cx) % TAB_W;
      insert_bytes(ed, blanks, spaces);
      set_want(ed);
      break;
   case KEY_C_Y: ed_command(ed, CMD_DELLINE); break;
   case KEY_C_O: ed_command(ed, CMD_OPEN); break;
   case KEY_A_F3: ed_command(ed, CMD_CLOSE); break;
   case KEY_MOUSE: ed_mouse(ed); break;
   case KEY_C_S: ed_command(ed, CMD_SAVE); break;
   case KEY_A_X: ed_command(ed, CMD_EXIT); break;
   case KEY_F9:
      ed->ime.mode = (unsigned char) ((ed->ime.mode + 1) % IME_MODES);
      break;
   case KEY_F10: ed_command(ed, menu_run(ed, 0)); break;
   case KEY_A_F: ed_command(ed, menu_run(ed, 0)); break;
   case KEY_A_E: ed_command(ed, menu_run(ed, 1)); break;
   case KEY_A_S: ed_command(ed, menu_run(ed, 2)); break;
   case KEY_A_O: ed_command(ed, menu_run(ed, 3)); break;
   case KEY_A_H: ed_command(ed, menu_run(ed, 4)); break;
   default:
      if (key >= 0x20 && key < 0x7f) {
         type_key(ed, key);
         return;
      }
   }
   ime_reset(&ed->ime);
}
