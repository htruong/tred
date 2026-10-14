// Host test for the IME: cc -I../src t_ime.c ../src/ime.c ../src/utf8.c
#include <stdio.h>
#include <string.h>
#include "ime.h"
#include "utf8.h"

static int fails;
static int w_alone;

// Type keys into an empty word the way the editor does, return UTF-8 text
static void type(int mode, int newstyle, const char *keys, char *res) {
   ime_t im;
   unsigned short text[64], out[IME_MAXWORD + 2];
   int n = 0, ws, nout, i;
   char *r = res;

   ime_init(&im);
   im.mode = mode;
   im.newstyle = newstyle;
   im.w_alone = (unsigned char) w_alone;
   for (; *keys; keys++) {
      for (ws = n; ws > 0 && n - ws < IME_MAXWORD && ime_word_char(&im, text[ws - 1]); ws--);
      if (ime_key(&im, text + ws, n - ws, *keys, out, &nout)) {
         for (i = 0; i < nout; i++) text[ws + i] = out[i];
         n = ws + nout;
      } else {
         if (!(*keys >= 'a' && *keys <= 'z') && !(*keys >= 'A' && *keys <= 'Z') && !(*keys >= '0' && *keys <= '9')) ime_reset(&im);
         text[n++] = (unsigned short) *keys;
      }
   }
   for (i = 0; i < n; i++) r += utf8_enc(text[i], r);
   *r = 0;
}

static void check(int mode, int newstyle, const char *keys, const char *want) {
   char got[256];

   type(mode, newstyle, keys, got);
   if (strcmp(got, want)) {
      printf("FAIL %-6s %-18s got %-14s want %s\n", ime_name(mode), keys, got, want);
      fails++;
   }
}

int main(void) {
   static const char *telex[][2] = {
      {"tieengs", "tiếng"}, {"Vieetj", "Việt"}, {"nguwowif", "người"}, {"dduwowngf", "đường"},
      {"hoaf", "hòa"}, {"hoafn", "hoàn"}, {"quas", "quá"}, {"gias", "giá"}, {"gif", "gì"},
      {"thuyr", "thủy"}, {"khuyeens", "khuyến"}, {"ass", "as"}, {"aaa", "aa"}, {"ddd", "dd"},
      {"muwa", "mưa"}, {"hoawcj", "hoặc"}, {"thuowr", "thuở"}, {"tana", "tân"},
      {"TRUWOWNGF", "TRƯỜNG"}, {"yeeu", "yêu"}, {"w", "w"}, {"what", "what"}, {"Tieengs Vieetj", "Tiếng Việt"},
      {"ruwowuj", "rượu"}, {"nuwowcs", "nước"}, {"khoer", "khỏe"}, {"oai", "oai"}, {"ngoaif", "ngoài"},
      {"chuyeenj", "chuyện"}, {"Ddaij hocj", "Đại học"}, {"hoz", "hoz"}, {"hosz", "ho"},
      {"the", "the"}, {"fans", "fans"}, {"hello", "hello"}, {"xin chaof", "xin chào"},
      {"cuar", "của"}, {"giuwax", "giữa"}, {"quoocs", "quốc"}, {"tuwj", "tự"}, {"thw", "thw"},
      {"assa", "asa"}, {"hoafng", "hoàng"}, {"tuyeetj", "tuyệt"}, {"huwu", "hưu"}, {"khuyur", "khuỷu"},
      {0, 0}
   };
   static const char *vni[][2] = {
      {"tie61ng", "tiếng"}, {"d9u7o72ng", "đường"}, {"Vie65t", "Việt"}, {"hoa2", "hòa"},
      {"a11", "a1"}, {"mu7a", "mưa"}, {"a8", "ă"}, {"123", "123"}, {0, 0}
   };
   int i;

   for (i = 0; telex[i][0]; i++) check(IME_TELEX, 0, telex[i][0], telex[i][1]);
   for (i = 0; vni[i][0]; i++) check(IME_VNI, 0, vni[i][0], vni[i][1]);
   check(IME_TELEX, 1, "hoaf", "hoà");
   check(IME_TELEX, 1, "thuyr", "thuỷ");
   check(IME_TELEX, 1, "hoafn", "hoàn");
   check(IME_OFF, 0, "tieengs", "tieengs");
   w_alone = 1;
   check(IME_TELEX, 0, "w", "ư");
   check(IME_TELEX, 0, "W", "Ư");
   check(IME_TELEX, 0, "thw", "thư");
   check(IME_TELEX, 0, "nguwowif", "người");
   w_alone = 0;
   printf("%s (%d failures)\n", fails ? "FAILED" : "ok", fails);
   return fails != 0;
}
