#ifndef TEXTBUF_H
#define TEXTBUF_H

// Text as an array of lines of raw UTF-8 bytes (no line terminators).
// Each line and the line table stay under 64 KB so far pointers suffice.

#define TB_MAXLINES 8000
#define TB_MAXLEN   32000

#define TB_OK      0
#define TB_ENOMEM  1
#define TB_ETOOBIG 2
#define TB_EIO     3

typedef struct {
   char *p;
   unsigned short len, cap;
} line_t;

typedef struct {
   line_t *ln;
   unsigned short n, cap;
   unsigned char crlf, final_eol, bom, modified;
   char name[80];
} textbuf_t;

int tb_init(textbuf_t *tb);
void tb_free(textbuf_t *tb);
int tb_load(textbuf_t *tb, const char *path);
int tb_save(textbuf_t *tb, const char *path);
int tb_insert(textbuf_t *tb, unsigned l, unsigned off, const char *s, unsigned n);
int tb_delete(textbuf_t *tb, unsigned l, unsigned off, unsigned n);
int tb_split(textbuf_t *tb, unsigned l, unsigned off);
int tb_join(textbuf_t *tb, unsigned l);
int tb_delete_line(textbuf_t *tb, unsigned l);

#endif // TEXTBUF_H
