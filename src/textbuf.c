// Line-array text buffer holding UTF-8 bytes, with UTF-8 file load/save that
// keeps the file's line endings, BOM and final newline as they were.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port.h"
#include "textbuf.h"

static int line_reserve(line_t *ln, unsigned need) {
   unsigned cap;
   char *p;

   if (need <= ln->cap) return TB_OK;
   if (need > TB_MAXLEN) return TB_ETOOBIG;
   cap = (need + need / 4 + 16) & ~15u;
   if (cap > TB_MAXLEN) cap = TB_MAXLEN;
   p = (char *) realloc(ln->p, cap);
   if (!p) return TB_ENOMEM;
   ln->p = p;
   ln->cap = (unsigned short) cap;
   return TB_OK;
}

// Make room for a new empty line at index l
static int lines_open(textbuf_t *tb, unsigned l) {
   unsigned cap;
   line_t *t;

   if (tb->n >= TB_MAXLINES) return TB_ETOOBIG;
   if (tb->n == tb->cap) {
      cap = tb->cap + tb->cap / 2 + 64;
      if (cap > TB_MAXLINES) cap = TB_MAXLINES;
      t = (line_t *) realloc(tb->ln, cap * sizeof(line_t));
      if (!t) return TB_ENOMEM;
      tb->ln = t;
      tb->cap = (unsigned short) cap;
   }
   memmove(tb->ln + l + 1, tb->ln + l, (tb->n - l) * sizeof(line_t));
   tb->ln[l].p = 0;
   tb->ln[l].len = tb->ln[l].cap = 0;
   tb->n++;
   return TB_OK;
}

int tb_init(textbuf_t *tb) {
   tb->ln = 0;
   tb->n = tb->cap = 0;
   tb->crlf = 1;
   tb->final_eol = 1;
   tb->bom = 0;
   tb->modified = 0;
   tb->name[0] = 0;
   return lines_open(tb, 0);
}

void tb_free(textbuf_t *tb) {
   unsigned i;

   for (i = 0; i < tb->n; i++) free(tb->ln[i].p);
   free(tb->ln);
   tb->ln = 0;
   tb->n = tb->cap = 0;
}

int tb_load(textbuf_t *tb, const char *path) {
   static char block[4096];
   FILE *f;
   unsigned got, i, start, cur = 0, first = 1, seen_eol = 0;
   int err = TB_OK;
   line_t *ln;

   tb_free(tb);
   if ((err = tb_init(tb)) != TB_OK) return err;
   f = fopen(path, "rb");
   if (!f) return TB_EIO;

   while (err == TB_OK && (got = (unsigned) fread(block, 1, sizeof(block), f)) > 0) {
      start = 0;
      if (first && got >= 3 && (unsigned char) block[0] == 0xef && (unsigned char) block[1] == 0xbb && (unsigned char) block[2] == 0xbf) {
         tb->bom = 1;
         start = 3;
      }
      first = 0;
      for (i = start; i <= got && err == TB_OK; i++) {
         if (i < got && block[i] != '\n') continue;
         // block[start..i) belongs to the current line
         ln = &tb->ln[cur];
         if (i > start) {
            if ((err = line_reserve(ln, ln->len + (i - start))) != TB_OK) break;
            memcpy(ln->p + ln->len, block + start, i - start);
            ln->len += (unsigned short) (i - start);
         }
         if (i < got) {
            if (!seen_eol) tb->crlf = ln->len > 0 && ln->p[ln->len - 1] == '\r';
            seen_eol = 1;
            if (ln->len > 0 && ln->p[ln->len - 1] == '\r') ln->len--;
            cur++;
            err = lines_open(tb, cur);
         }
         start = i + 1;
      }
   }
   if (ferror(f)) err = TB_EIO;
   fclose(f);

   ln = &tb->ln[tb->n - 1];
   if (ln->len > 0 && ln->p[ln->len - 1] == 0x1a) ln->len--; // DOS EOF marker
   if (tb->n > 1 && ln->len == 0) {
      // The file ended with a newline: drop the empty line after it
      free(ln->p);
      tb->n--;
      tb->final_eol = 1;
   } else {
      tb->final_eol = 0;
   }
   if (err != TB_OK) {
      tb_free(tb);
      tb_init(tb);
   }
   tb->modified = 0;
   return err;
}

int tb_save(textbuf_t *tb, const char *path) {
   static char tmp[84];
   FILE *f;
   unsigned i;
   int ok = 1;
   char *dot;

   strcpy(tmp, path);
   dot = strrchr(tmp, '.');
   if (dot && !strchr(dot, '\\') && !strchr(dot, '/')) *dot = 0;
   strcat(tmp, ".$$$");

   f = fopen(tmp, "wb");
   if (!f) return TB_EIO;
   if (tb->bom) ok = fwrite("\xef\xbb\xbf", 1, 3, f) == 3;
   for (i = 0; ok && i < tb->n; i++) {
      if (tb->ln[i].len) ok = fwrite(tb->ln[i].p, 1, tb->ln[i].len, f) == tb->ln[i].len;
      if (ok && (i + 1 < tb->n || tb->final_eol)) {
         ok = tb->crlf ? fwrite("\r\n", 1, 2, f) == 2 : fwrite("\n", 1, 1, f) == 1;
      }
   }
   if (fclose(f) != 0) ok = 0;
   if (!ok) {
      remove(tmp);
      return TB_EIO;
   }
   remove(path);
   if (rename(tmp, path) != 0) return TB_EIO;
   tb->modified = 0;
   return TB_OK;
}

int tb_insert(textbuf_t *tb, unsigned l, unsigned off, const char *s, unsigned n) {
   line_t *ln = &tb->ln[l];
   int err;

   if (n == 0) return TB_OK;
   if ((err = line_reserve(ln, ln->len + n)) != TB_OK) return err;
   memmove(ln->p + off + n, ln->p + off, ln->len - off);
   memcpy(ln->p + off, s, n);
   ln->len += (unsigned short) n;
   tb->modified = 1;
   return TB_OK;
}

int tb_delete(textbuf_t *tb, unsigned l, unsigned off, unsigned n) {
   line_t *ln = &tb->ln[l];

   if (n == 0) return TB_OK;
   memmove(ln->p + off, ln->p + off + n, ln->len - off - n);
   ln->len -= (unsigned short) n;
   tb->modified = 1;
   return TB_OK;
}

int tb_split(textbuf_t *tb, unsigned l, unsigned off) {
   line_t *a, *b;
   int err;

   if ((err = lines_open(tb, l + 1)) != TB_OK) return err;
   a = &tb->ln[l];
   b = &tb->ln[l + 1];
   if ((err = line_reserve(b, a->len - off)) != TB_OK) {
      memmove(tb->ln + l + 1, tb->ln + l + 2, (tb->n - l - 2) * sizeof(line_t));
      tb->n--;
      return err;
   }
   if (a->len > off) memcpy(b->p, a->p + off, a->len - off);
   b->len = (unsigned short) (a->len - off);
   a->len = (unsigned short) off;
   tb->modified = 1;
   return TB_OK;
}

int tb_join(textbuf_t *tb, unsigned l) {
   line_t *a = &tb->ln[l], *b = &tb->ln[l + 1];
   int err;

   if (l + 1 >= tb->n) return TB_OK;
   if ((err = line_reserve(a, a->len + b->len)) != TB_OK) return err;
   if (b->len) memcpy(a->p + a->len, b->p, b->len);
   a->len += b->len;
   return tb_delete_line(tb, l + 1);
}

int tb_delete_line(textbuf_t *tb, unsigned l) {
   if (tb->n == 1) {
      tb->ln[0].len = 0;
   } else {
      free(tb->ln[l].p);
      memmove(tb->ln + l, tb->ln + l + 1, (tb->n - l - 1) * sizeof(line_t));
      tb->n--;
   }
   tb->modified = 1;
   return TB_OK;
}
