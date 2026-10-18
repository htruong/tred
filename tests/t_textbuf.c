// Host test for textbuf: byte-identical load/save round trips and edits
#include <stdio.h>
#include <string.h>
#include "textbuf.h"

static int fails;

static void roundtrip(const char *name, const char *data, unsigned len) {
   static char back[8192];
   FILE *f;
   textbuf_t tb;
   unsigned got;

   f = fopen("t.txt", "wb");
   fwrite(data, 1, len, f);
   fclose(f);
   tb_init(&tb);
   if (tb_load(&tb, "t.txt") != TB_OK || tb_save(&tb, "t.txt") != TB_OK) {
      printf("FAIL %s: io\n", name);
      fails++;
      return;
   }
   f = fopen("t.txt", "rb");
   got = (unsigned) fread(back, 1, sizeof(back), f);
   fclose(f);
   if (got != len || memcmp(back, data, len)) {
      printf("FAIL %s: %u bytes back, want %u\n", name, got, len);
      fails++;
   }
   tb_free(&tb);
}

int main(void) {
   static char big[6000];
   textbuf_t tb;
   unsigned i;

   roundtrip("lf", "ab\nc\n", 5);
   roundtrip("crlf", "ab\r\ncd\r\n", 8);
   roundtrip("no final eol", "ab\ncd", 5);
   roundtrip("empty", "", 0);
   roundtrip("blank lines", "\n\n\n", 3);
   roundtrip("bom", "\xef\xbb\xbfVi\xe1\xbb\x87t\r\n", 10);
   roundtrip("invalid utf8", "\xff\xfe\x80z\n", 5);
   for (i = 0; i < sizeof(big); i++) big[i] = (i % 61 == 60) ? '\n' : 'a' + i % 26;
   roundtrip("spans blocks", big, sizeof(big));

   tb_init(&tb);
   tb_insert(&tb, 0, 0, "hello world", 11);
   tb_split(&tb, 0, 5);
   tb_delete(&tb, 1, 0, 1);
   tb_join(&tb, 0);
   if (tb.n != 1 || tb.ln[0].len != 10 || memcmp(tb.ln[0].p, "helloworld", 10)) {
      printf("FAIL edits\n");
      fails++;
   }
   tb_free(&tb);
   remove("t.txt");
   printf("%s (%d failures)\n", fails ? "FAILED" : "ok", fails);
   return fails != 0;
}
