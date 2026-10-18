// edit.com style menu bar, drop-down menus and dialog boxes. Each modal
// widget saves the cells under it and puts them back when it closes, so the
// editor never has to redraw underneath.

#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <dos.h>
#include "menu.h"
#include "surface.h"
#include "draw.h"
#include "kbd.h"
#include "vgatxt.h"

#define A_BAR     0x70
#define A_BAR_SEL 0x07
#define A_HOT     (0x70 | COLOR_WHITE)
#define A_HOT_SEL (0x00 | COLOR_WHITE)
#define A_SHADOW  COLOR_DARKGRAY
#define A_DIALOG  0x70
#define A_FIELD   0x07

typedef struct {
   const char *label; // '&' marks the hotkey; "-" is a separator
   const char *keys;
   int cmd;
} mitem_t;

typedef struct {
   const char *title;
   int x;
   const mitem_t *items;
} menu_t;

static const mitem_t file_items[] = {
   {"&Open...", "Ctrl+O", CMD_OPEN}, {"&Close", "Alt+F3", CMD_CLOSE}, {"&Save", "Ctrl+S", CMD_SAVE}, {"Save &As...", "", CMD_SAVEAS}, {"-", 0, 0},
   {"E&xit", "Alt+X", CMD_EXIT}, {0, 0, 0}
};
static const mitem_t edit_items[] = {{"&Delete Line", "Ctrl+Y", CMD_DELLINE}, {0, 0, 0}};
static const mitem_t search_items[] = {{"&Go To Line...", "", CMD_GOTO}, {0, 0, 0}};
static const mitem_t option_items[] = {
   {"&Telex", "", CMD_IME_TELEX}, {"&VNI", "", CMD_IME_VNI}, {"&Off", "", CMD_IME_OFF},
   {"-", 0, 0}, {"&New Tone Style (hoà)", "", CMD_TONESTYLE}, {"&Lone w Types ư", "", CMD_WALONE},
   {"&Word Wrap", "", CMD_WRAP}, {"-", 0, 0},
   {"Switch Input Method", "F9", CMD_NONE}, {0, 0, 0}
};
static const mitem_t help_items[] = {{"&About...", "", CMD_ABOUT}, {0, 0, 0}};

#define NMENUS 5
static const menu_t menus[NMENUS] = {
   {"&File", 2, file_items}, {"&Edit", 8, edit_items}, {"&Search", 14, search_items},
   {"&Options", 22, option_items}, {"&Help", 74, help_items}
};

static const view_t screen = {0, 0, 80, 25};

static int label_len(const char *s) {
   int n = 0;
   const unsigned char *p = (const unsigned char *) s;

   for (; *p; p++) {
      if (*p != '&' && (*p & 0xc0) != 0x80) n++;
   }
   return n;
}

// Draw a label, its '&' letter in the hotkey colour; returns the hotkey
static int draw_label(int x, int y, const char *s, unsigned char a, unsigned char hot) {
   char one[4];
   int hk = 0, n, len;
   const unsigned char *p = (const unsigned char *) s;

   while (*p) {
      if (*p == '&') {
         hk = p[1] | 0x20;
         p++;
         one[0] = (char) *p;
         one[1] = 0;
         v_text(&screen, x++, y, one, hot);
         p++;
         continue;
      }
      len = *p < 0x80 ? 1 : *p < 0xe0 ? 2 : 3;
      memcpy(one, p, len);
      one[len] = 0;
      n = v_utf8(&screen, x, y, one, a);
      x += n;
      p += len;
   }
   return hk;
}

void menu_bar_draw(int open) {
   int i;

   v_fill(&screen, 0, 0, 80, 1, ' ', A_BAR);
   for (i = 0; i < NMENUS; i++) {
      if (i == open) {
         v_fill(&screen, menus[i].x - 1, 0, label_len(menus[i].title) + 2, 1, ' ', A_BAR_SEL);
         draw_label(menus[i].x, 0, menus[i].title, A_BAR_SEL, A_HOT_SEL);
      } else {
         draw_label(menus[i].x, 0, menus[i].title, A_BAR, open >= 0 ? A_HOT : A_BAR);
      }
   }
}

int menu_hit(int x) {
   int i;

   for (i = 0; i < NMENUS; i++) {
      if (x >= menus[i].x - 1 && x <= menus[i].x + label_len(menus[i].title)) return i;
   }
   return -1;
}

static void shadow(int x, int y, int w, int h) {
   scr_shade(x + w, y + 1, 2, h, A_SHADOW);
   scr_shade(x + 2, y + h, w, 1, A_SHADOW);
}

static void box_geometry(int m, int *x, int *w, int *h) {
   const mitem_t *it = menus[m].items;
   int i, lw = 0, kw = 0;

   for (i = 0; it[i].label; i++) {
      if (it[i].label[0] == '-') continue;
      if (label_len(it[i].label) > lw) lw = label_len(it[i].label);
      if ((int) strlen(it[i].keys) > kw) kw = (int) strlen(it[i].keys);
   }
   *w = lw + (kw ? kw + 3 : 0) + 5;
   *h = i + 2;
   *x = menus[m].x - 2;
   if (*x + *w + 2 > 80) *x = 80 - *w - 2;
}

static void draw_box(editor_t *ed, int m, int sel) {
   const mitem_t *it = menus[m].items;
   int x, w, h, i;
   unsigned char a;
   view_t box;

   box_geometry(m, &x, &w, &h);
   box.x = x;
   box.y = 1;
   box.w = w;
   box.h = h;
   v_fill(&box, 0, 0, w, h, ' ', A_BAR);
   v_frame(&box, A_BAR);
   for (i = 0; it[i].label; i++) {
      if (it[i].label[0] == '-') {
         v_put(&box, 0, i + 1, 0xc3, A_BAR);
         v_fill(&box, 1, i + 1, w - 2, 1, 0xc4, A_BAR);
         v_put(&box, w - 1, i + 1, 0xb4, A_BAR);
         continue;
      }
      a = i == sel ? A_BAR_SEL : A_BAR;
      v_fill(&box, 1, i + 1, w - 2, 1, ' ', a);
      if (ed_menu_checked(ed, it[i].cmd)) v_put(&box, 1, i + 1, 0x07, a);
      draw_label(x + 2, 2 + i, it[i].label, a, i == sel ? A_HOT_SEL : A_HOT);
      v_text(&box, w - 2 - (int) strlen(it[i].keys), i + 1, it[i].keys, a);
   }
   shadow(x, 1, w, h);
}

static int next_item(const mitem_t *it, int sel, int dir) {
   int n, i;

   for (n = 0; it[n].label; n++);
   for (i = 0; i < n; i++) {
      sel = (sel + dir + n) % n;
      if (it[sel].label[0] != '-') return sel;
   }
   return sel;
}

static int hotkey_of(const char *label) {
   const char *p = strchr(label, '&');

   return p ? p[1] | 0x20 : 0;
}

int menu_run(editor_t *ed, int m) {
   cell_t *saved = 0;
   int sel = 0, k, x, w, h, i, cmd = -1;
   const mitem_t *it;

   ed_render(ed); // keys typed ahead may not be on screen yet
   vga_cursor_shape(CURSOR_HIDDEN, 0);
   while (cmd < 0) {
      it = menus[m].items;
      box_geometry(m, &x, &w, &h);
      if (!saved) saved = scr_save(x, 1, w + 2, h + 1);
      menu_bar_draw(m);
      draw_box(ed, m, sel);
      scr_flush();
      k = kbd_get();
      switch (k) {
      case KEY_LEFT:
      case KEY_RIGHT:
         scr_restore(x, 1, w + 2, h + 1, saved);
         saved = 0;
         m = (m + (k == KEY_LEFT ? NMENUS - 1 : 1)) % NMENUS;
         sel = 0;
         break;
      case KEY_UP: sel = next_item(it, sel, -1); break;
      case KEY_DOWN: sel = next_item(it, sel, 1); break;
      case KEY_ENTER: cmd = it[sel].cmd; break;
      case KEY_ESC: case KEY_F10: case KEY_SCRIPT_END: cmd = CMD_NONE; break;
      case KEY_MOUSE:
         if (mouse_ev.wheel) break;
         if (mouse_ev.y == 0) {
            // On the bar: switch menus; a press elsewhere on it closes
            i = menu_hit(mouse_ev.x);
            if (i >= 0 && i != m) {
               scr_restore(x, 1, w + 2, h + 1, saved);
               saved = 0;
               m = i;
               sel = 0;
            } else if (i < 0 && mouse_ev.pressed) {
               cmd = CMD_NONE;
            }
         } else if (mouse_ev.x > x && mouse_ev.x < x + w - 1 && mouse_ev.y > 1 && mouse_ev.y < h) {
            // Items highlight under the pointer and are chosen on release
            i = mouse_ev.y - 2;
            if (it[i].label[0] != '-') {
               sel = i;
               if (mouse_ev.released) cmd = it[i].cmd;
            }
         } else if (mouse_ev.pressed) {
            cmd = CMD_NONE;
         }
         break;
      default:
         for (i = 0; it[i].label; i++) {
            if (k < 0x80 && hotkey_of(it[i].label) == (k | 0x20)) cmd = it[i].cmd;
         }
      }
   }
   scr_restore(x, 1, w + 2, h + 1, saved);
   menu_bar_draw(-1);
   vga_cursor_shape(ed->ins ? CURSOR_INSERT : CURSOR_OVERWRITE, CURSOR_END);
   return cmd;
}

static int utf8_len(const char *s) {
   int n = 0;

   for (; *s; s++) {
      if (((unsigned char) *s & 0xc0) != 0x80) n++;
   }
   return n;
}

// A centred dialog frame; returns the view of its inside
static void dialog(const char *title, int w, int h, view_t *v, cell_t **saved) {
   int tl;

   v->w = w;
   v->h = h;
   v->x = (80 - w) / 2;
   v->y = (25 - h) / 2;
   *saved = scr_save(v->x, v->y, w + 2, h + 1);
   v_fill(v, 0, 0, w, h, ' ', A_DIALOG);
   v_frame(v, A_DIALOG);
   if (title) {
      tl = utf8_len(title);
      v_fill(v, (w - tl) / 2 - 1, 0, tl + 2, 1, ' ', A_DIALOG);
      v_utf8(v, (w - tl) / 2, 0, title, A_DIALOG);
   }
   v_put(v, 0, h - 3, 0xc3, A_DIALOG);
   v_fill(v, 1, h - 3, w - 2, 1, 0xc4, A_DIALOG);
   v_put(v, w - 1, h - 3, 0xb4, A_DIALOG);
   shadow(v->x, v->y, w, h);
}

// sel is the focused button, or -1 for none
static void draw_buttons(const view_t *v, const char * const *buttons, int n, int sel) {
   int i, total = 0, x;

   for (i = 0; i < n; i++) total += (int) strlen(buttons[i]) + 4 + (i ? 3 : 0);
   x = (v->w - total) / 2;
   for (i = 0; i < n; i++) {
      v_put(v, x, v->h - 2, '<', i == sel ? A_HOT : A_DIALOG);
      v_text(v, x + 2, v->h - 2, buttons[i], A_DIALOG);
      x += (int) strlen(buttons[i]) + 3;
      v_put(v, x, v->h - 2, '>', i == sel ? A_HOT : A_DIALOG);
      x += 4;
   }
}

// Which button of draw_buttons is at screen cell (mx, my), or -1
static int button_at(const view_t *v, const char * const *buttons, int n, int mx, int my) {
   int i, total = 0, x, len;

   if (my != v->y + v->h - 2) return -1;
   for (i = 0; i < n; i++) total += (int) strlen(buttons[i]) + 4 + (i ? 3 : 0);
   x = v->x + (v->w - total) / 2;
   for (i = 0; i < n; i++) {
      len = (int) strlen(buttons[i]);
      if (mx >= x && mx <= x + len + 3) return i;
      x += len + 7;
   }
   return -1;
}

int msgbox(const char *title, const char * const *lines, const char * const *buttons, int nbtn, int def) {
   view_t v;
   cell_t *saved;
   int w = 30, h, n, i, k, sel = def, done = -2;

   for (n = 0; lines[n]; n++) {
      if (utf8_len(lines[n]) + 6 > w) w = utf8_len(lines[n]) + 6;
   }
   h = n + 5;
   dialog(title, w, h, &v, &saved);
   for (i = 0; i < n; i++) v_utf8(&v, (w - utf8_len(lines[i])) / 2, i + 1, lines[i], A_DIALOG);
   vga_cursor_shape(CURSOR_HIDDEN, 0);
   while (done == -2) {
      draw_buttons(&v, buttons, nbtn, sel);
      scr_flush();
      k = kbd_get();
      if (k == KEY_MOUSE && mouse_ev.pressed && (i = button_at(&v, buttons, nbtn, mouse_ev.x, mouse_ev.y)) >= 0) done = i;
      else if (k == KEY_TAB || k == KEY_RIGHT) sel = (sel + 1) % nbtn;
      else if (k == KEY_LEFT) sel = (sel + nbtn - 1) % nbtn;
      else if (k == KEY_ENTER) done = sel;
      else if (k == KEY_ESC || k == KEY_SCRIPT_END) done = -1;
      else for (i = 0; i < nbtn; i++) {
         if (k < 0x80 && (buttons[i][0] | 0x20) == (k | 0x20)) done = i;
      }
   }
   scr_restore(v.x, v.y, w + 2, h + 1, saved);
   return done;
}

int inputbox(const char *title, const char *prompt, char *buf, int max) {
   static const char * const buttons[] = {"OK", "Cancel"};
   view_t v;
   cell_t *saved;
   int w = 60, h = 7, len = (int) strlen(buf), k, b, done = -1, fx = 3 + (int) strlen(prompt);

   dialog(title, w, h, &v, &saved);
   v_text(&v, 2, 2, prompt, A_DIALOG);
   draw_buttons(&v, buttons, 2, 0);
   vga_cursor_shape(CURSOR_INSERT, CURSOR_END);
   while (done < 0) {
      v_fill(&v, fx, 2, w - fx - 2, 1, ' ', A_FIELD);
      v_text(&v, fx, 2, buf, A_FIELD);
      scr_flush();
      vga_cursor(v.x + fx + len, v.y + 2);
      k = kbd_get();
      if (k == KEY_MOUSE && mouse_ev.pressed && (b = button_at(&v, buttons, 2, mouse_ev.x, mouse_ev.y)) >= 0) done = b == 0;
      else if (k == KEY_ENTER) done = 1;
      else if (k == KEY_ESC || k == KEY_SCRIPT_END) done = 0;
      else if (k == KEY_BS && len > 0) buf[--len] = 0;
      else if (k >= 0x20 && k < 0x7f && len < max && fx + len < w - 3) {
         buf[len++] = (char) k;
         buf[len] = 0;
      }
   }
   scr_restore(v.x, v.y, w + 2, h + 1, saved);
   return done;
}

// ---- File Open ----

#define MAX_ENTRIES 256
#define LIST_ROWS 11

static char entries[MAX_ENTRIES][13];
static int n_entries, n_dirs;

static int name_cmp(const void *a, const void *b) {
   return strcmp((const char *) a, (const char *) b);
}

// Directories (except ".") first, then files matching pattern, each sorted
static void read_dir(const char *pattern) {
   struct find_t f;

   n_entries = 0;
   if (_dos_findfirst("*.*", _A_SUBDIR, &f) == 0) {
      do {
         if ((f.attrib & _A_SUBDIR) && strcmp(f.name, ".") && n_entries < MAX_ENTRIES) {
            strcpy(entries[n_entries++], f.name);
         }
      } while (_dos_findnext(&f) == 0);
   }
   n_dirs = n_entries;
   if (_dos_findfirst(pattern, _A_NORMAL | _A_RDONLY | _A_ARCH, &f) == 0) {
      do {
         if (!(f.attrib & _A_SUBDIR) && n_entries < MAX_ENTRIES) strcpy(entries[n_entries++], f.name);
      } while (_dos_findnext(&f) == 0);
   }
   qsort(entries, n_dirs, sizeof(entries[0]), name_cmp);
   qsort(entries + n_dirs, n_entries - n_dirs, sizeof(entries[0]), name_cmp);
}

static int is_dir(const char *name) {
   struct find_t f;

   return _dos_findfirst(name, _A_SUBDIR, &f) == 0 && (f.attrib & _A_SUBDIR);
}

int open_dialog(char *name, int max) {
   static const char * const buttons[] = {"OK", "Cancel"};
   char field[64], pattern[64], cwd[68], start[68], item[16];
   view_t v, list;
   cell_t *saved;
   int w = 56, h = 20, focus = 0, sel = 0, first = 0, done = -1, k, i, len, fx = 13;
   int fresh = 1; // the field holds preset text that typing replaces
   int b;

   strcpy(pattern, "*.*");
   strcpy(field, pattern);
   if (!getcwd(start, sizeof(start))) start[0] = 0;
   read_dir(pattern);
   dialog("Open", w, h, &v, &saved);
   list.x = v.x + 2;
   list.y = v.y + 4;
   list.w = w - 4;
   list.h = LIST_ROWS + 2;
   v_text(&v, 2, 1, "File Name:", A_DIALOG);

   while (done < 0) {
      len = (int) strlen(field);
      v_fill(&v, fx, 1, w - fx - 2, 1, ' ', A_FIELD);
      v_text(&v, fx, 1, field, A_FIELD);
      if (!getcwd(cwd, sizeof(cwd))) cwd[0] = 0;
      v_fill(&v, 2, 2, w - 4, 1, ' ', A_DIALOG);
      v_text(&v, 2, 2, strlen(cwd) > (unsigned) (w - 4) ? cwd + strlen(cwd) - (w - 4) : cwd, A_DIALOG);

      // List of directories and files
      v_fill(&list, 0, 0, list.w, list.h, ' ', A_DIALOG);
      v_frame(&list, A_DIALOG);
      v_text(&list, 2, 0, " Files and Directories ", A_DIALOG);
      if (sel >= n_entries) sel = n_entries - 1;
      if (sel < 0) sel = 0;
      if (sel < first) first = sel;
      if (sel >= first + LIST_ROWS) first = sel - LIST_ROWS + 1;
      for (i = 0; i < LIST_ROWS && first + i < n_entries; i++) {
         strcpy(item, entries[first + i]);
         if (first + i < n_dirs) strcat(item, "\\");
         v_fill(&list, 1, i + 1, list.w - 2, 1, ' ', first + i == sel && focus == 1 ? A_BAR_SEL : A_DIALOG);
         v_text(&list, 2, i + 1, item, first + i == sel && focus == 1 ? A_BAR_SEL : A_DIALOG);
      }
      draw_buttons(&v, buttons, 2, focus >= 2 ? focus - 2 : -1);
      scr_flush();
      if (focus == 0) {
         vga_cursor_shape(CURSOR_INSERT, CURSOR_END);
         vga_cursor(v.x + fx + len, v.y + 1);
      } else {
         vga_cursor_shape(CURSOR_HIDDEN, 0);
      }

      k = kbd_get();
      if (k == KEY_MOUSE) {
         // Clicks become the key that does the same thing
         i = mouse_ev.y - list.y - 1;
         if (mouse_ev.wheel && n_entries > 0) {
            focus = 1;
            k = mouse_ev.wheel > 0 ? KEY_DOWN : KEY_UP;
         } else if (!mouse_ev.pressed) {
            continue;
         } else if ((b = button_at(&v, buttons, 2, mouse_ev.x, mouse_ev.y)) >= 0) {
            focus = 2 + b;
            k = KEY_ENTER;
         } else if (mouse_ev.y == v.y + 1 && mouse_ev.x >= v.x + fx) {
            focus = 0;
            continue;
         } else if (i >= 0 && i < LIST_ROWS && first + i < n_entries && mouse_ev.x > list.x && mouse_ev.x < list.x + list.w - 1) {
            if (focus == 1 && first + i == sel) {
               k = KEY_ENTER; // a second click on the highlighted entry opens it
            } else {
               focus = 1;
               sel = first + i;
               strcpy(field, sel >= n_dirs ? entries[sel] : pattern);
               fresh = 1;
               continue;
            }
         } else {
            continue;
         }
      }
      if (focus == 0 && fresh && (k == KEY_BS || (k > 0x20 && k < 0x7f))) {
         field[0] = 0;
         len = 0;
      }
      if (k != KEY_TAB) fresh = 0;
      if (k == KEY_ESC || k == KEY_SCRIPT_END) {
         done = 0;
      } else if (k == KEY_TAB) {
         focus = (focus + 1) % 4;
      } else if (focus == 1 && n_entries > 0 && (k == KEY_UP || k == KEY_DOWN || k == KEY_PGUP ||
                 k == KEY_PGDN || k == KEY_HOME || k == KEY_END)) {
         if (k == KEY_UP) sel--;
         else if (k == KEY_DOWN) sel++;
         else if (k == KEY_PGUP) sel -= LIST_ROWS;
         else if (k == KEY_PGDN) sel += LIST_ROWS;
         else if (k == KEY_HOME) sel = 0;
         else sel = n_entries - 1;
         if (sel < 0) sel = 0;
         if (sel >= n_entries) sel = n_entries - 1;
         // Like edit.com, the highlighted file goes into the name field
         strcpy(field, sel >= n_dirs ? entries[sel] : pattern);
         fresh = 1;
      } else if (k == KEY_ENTER && focus == 3) {
         done = 0;
      } else if (k == KEY_ENTER && focus == 1 && n_entries > 0) {
         if (sel < n_dirs) {
            if (chdir(entries[sel]) == 0) read_dir(pattern);
            sel = first = 0;
            strcpy(field, pattern);
            fresh = 1;
         } else {
            strcpy(field, entries[sel]);
            done = 1;
         }
      } else if (k == KEY_ENTER) {
         // The name field, or OK: a pattern filters, a directory is entered, a file is opened
         if (!field[0]) continue;
         if (strchr(field, '*') || strchr(field, '?')) {
            strcpy(pattern, field);
            read_dir(pattern);
            sel = first = 0;
            fresh = 1;
         } else if (is_dir(field)) {
            if (chdir(field) == 0) read_dir(pattern);
            strcpy(field, pattern);
            sel = first = 0;
            fresh = 1;
         } else {
            done = 1;
         }
      } else if (focus == 0 && k == KEY_BS && len > 0) {
         field[len - 1] = 0;
      } else if (focus == 0 && k > 0x20 && k < 0x7f && len < (int) sizeof(field) - 1 && fx + len < w - 3) {
         field[len] = (char) k;
         field[len + 1] = 0;
      }
   }
   scr_restore(v.x, v.y, w + 2, h + 1, saved);
   if (done && (int) strlen(field) < max) {
      strcpy(name, field);
      return 1;
   }
   if (start[0]) chdir(start); // cancelled: go back to where we were
   return 0;
}
