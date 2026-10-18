// Vietnamese input method: Unikey-style Telex and VNI.
//
// The current word is decomposed into letters (base, modifier, tone), the key
// is applied, the syllable's tone is placed on the right vowel again and the
// word is recomposed. The editor replaces the old word with the result.

#include "port.h"
#include "ime.h"

enum { M_NONE, M_HAT, M_BREVE, M_HORN, M_BAR };

typedef struct {
   unsigned char base; // lowercase ASCII letter, or the key itself for digits
   unsigned char mod;
   unsigned char tone; // 0 none, 1 sắc, 2 huyền, 3 hỏi, 4 ngã, 5 nặng
   unsigned char up;
} vch_t;

// [case][vowel][tone]; vowels a ă â e ê i o ô ơ u ư y
static const unsigned short vowels[2][12][6] = {
   {
      {0x0061, 0x00e1, 0x00e0, 0x1ea3, 0x00e3, 0x1ea1}, // aáàảãạ
      {0x0103, 0x1eaf, 0x1eb1, 0x1eb3, 0x1eb5, 0x1eb7}, // ăắằẳẵặ
      {0x00e2, 0x1ea5, 0x1ea7, 0x1ea9, 0x1eab, 0x1ead}, // âấầẩẫậ
      {0x0065, 0x00e9, 0x00e8, 0x1ebb, 0x1ebd, 0x1eb9}, // eéèẻẽẹ
      {0x00ea, 0x1ebf, 0x1ec1, 0x1ec3, 0x1ec5, 0x1ec7}, // êếềểễệ
      {0x0069, 0x00ed, 0x00ec, 0x1ec9, 0x0129, 0x1ecb}, // iíìỉĩị
      {0x006f, 0x00f3, 0x00f2, 0x1ecf, 0x00f5, 0x1ecd}, // oóòỏõọ
      {0x00f4, 0x1ed1, 0x1ed3, 0x1ed5, 0x1ed7, 0x1ed9}, // ôốồổỗộ
      {0x01a1, 0x1edb, 0x1edd, 0x1edf, 0x1ee1, 0x1ee3}, // ơớờởỡợ
      {0x0075, 0x00fa, 0x00f9, 0x1ee7, 0x0169, 0x1ee5}, // uúùủũụ
      {0x01b0, 0x1ee9, 0x1eeb, 0x1eed, 0x1eef, 0x1ef1}, // ưứừửữự
      {0x0079, 0x00fd, 0x1ef3, 0x1ef7, 0x1ef9, 0x1ef5}, // yýỳỷỹỵ
   },
   {
      {0x0041, 0x00c1, 0x00c0, 0x1ea2, 0x00c3, 0x1ea0}, // AÁÀẢÃẠ
      {0x0102, 0x1eae, 0x1eb0, 0x1eb2, 0x1eb4, 0x1eb6}, // ĂẮẰẲẴẶ
      {0x00c2, 0x1ea4, 0x1ea6, 0x1ea8, 0x1eaa, 0x1eac}, // ÂẤẦẨẪẬ
      {0x0045, 0x00c9, 0x00c8, 0x1eba, 0x1ebc, 0x1eb8}, // EÉÈẺẼẸ
      {0x00ca, 0x1ebe, 0x1ec0, 0x1ec2, 0x1ec4, 0x1ec6}, // ÊẾỀỂỄỆ
      {0x0049, 0x00cd, 0x00cc, 0x1ec8, 0x0128, 0x1eca}, // IÍÌỈĨỊ
      {0x004f, 0x00d3, 0x00d2, 0x1ece, 0x00d5, 0x1ecc}, // OÓÒỎÕỌ
      {0x00d4, 0x1ed0, 0x1ed2, 0x1ed4, 0x1ed6, 0x1ed8}, // ÔỐỒỔỖỘ
      {0x01a0, 0x1eda, 0x1edc, 0x1ede, 0x1ee0, 0x1ee2}, // ƠỚỜỞỠỢ
      {0x0055, 0x00da, 0x00d9, 0x1ee6, 0x0168, 0x1ee4}, // UÚÙỦŨỤ
      {0x01af, 0x1ee8, 0x1eea, 0x1eec, 0x1eee, 0x1ef0}, // ƯỨỪỬỮỰ
      {0x0059, 0x00dd, 0x1ef2, 0x1ef6, 0x1ef8, 0x1ef4}, // YÝỲỶỸỴ
   }
};
static const unsigned char vowel_base[12] = {'a', 'a', 'a', 'e', 'e', 'i', 'o', 'o', 'o', 'u', 'u', 'y'};
static const unsigned char vowel_mod[12] = {M_NONE, M_BREVE, M_HAT, M_NONE, M_HAT, M_NONE, M_NONE, M_HAT, M_HORN, M_NONE, M_HORN, M_NONE};

static const char * const initials[] = {
   "", "b", "c", "ch", "d", "D", "g", "gh", "gi", "h", "k", "kh", "l", "m", "n", "ng", "ngh",
   "nh", "p", "ph", "qu", "r", "s", "t", "th", "tr", "v", "x", 0
};
static const char * const finals[] = {"", "c", "ch", "m", "n", "ng", "nh", "p", "t", 0};

void ime_init(ime_t *im) {
   im->mode = IME_TELEX;
   im->newstyle = 0;
   im->w_alone = 0;
   im->suppress = 0;
}

void ime_reset(ime_t *im) {
   im->suppress = 0;
}

const char *ime_name(int mode) {
   return mode == IME_TELEX ? "TELEX" : mode == IME_VNI ? "VNI" : "OFF";
}

static int decompose(unsigned cp, vch_t *v) {
   int up, i, t;

   v->mod = M_NONE;
   v->tone = 0;
   v->up = 0;
   if ((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z')) {
      v->up = cp <= 'Z';
      v->base = (unsigned char) (cp | 0x20);
      return 1;
   }
   if (cp == 0x111 || cp == 0x110) {
      v->base = 'd';
      v->mod = M_BAR;
      v->up = cp == 0x110;
      return 1;
   }
   if (cp < 0xc0) return 0;
   for (up = 0; up < 2; up++) {
      for (i = 0; i < 12; i++) {
         for (t = 0; t < 6; t++) {
            if (vowels[up][i][t] == cp) {
               v->base = vowel_base[i];
               v->mod = vowel_mod[i];
               v->tone = (unsigned char) t;
               v->up = (unsigned char) up;
               return 1;
            }
         }
      }
   }
   return 0;
}

static unsigned short compose(const vch_t *v) {
   int i;

   if (v->base == 'd' && v->mod == M_BAR) return v->up ? 0x110 : 0x111;
   for (i = 0; i < 12; i++) {
      if (vowel_base[i] == v->base && vowel_mod[i] == v->mod) return vowels[v->up][i][v->tone];
   }
   return v->up && v->base >= 'a' && v->base <= 'z' ? v->base - 32 : v->base;
}

int ime_word_char(const ime_t *im, unsigned cp) {
   vch_t v;

   if (im->mode == IME_VNI && cp >= '0' && cp <= '9') return 1;
   return decompose(cp, &v);
}

static int is_vowel(const vch_t *v) {
   return v->base == 'a' || v->base == 'e' || v->base == 'i' || v->base == 'o' || v->base == 'u' || v->base == 'y';
}

static int in_list(const char * const *list, const vch_t *w, int from, int to) {
   int i, j;

   for (i = 0; list[i]; i++) {
      for (j = 0; from + j < to && list[i][j]; j++) {
         if (list[i][j] != (w[from + j].mod == M_BAR ? 'D' : w[from + j].base)) break;
      }
      if (from + j == to && !list[i][j]) return 1;
   }
   return 0;
}

// Split w into initial [0,vs), vowels [vs,ve), final [ve,n); 0 if not a syllable
static int parse(const vch_t *w, int n, int *vs, int *ve) {
   int i = 0;

   while (i < n && !is_vowel(&w[i])) {
      if (w[i].base < 'a' || w[i].base > 'z') return 0;
      i++;
   }
   // "qu" and "gi" (when another vowel follows) belong to the initial
   if (i == 1 && i < n && w[0].base == 'q' && w[i].base == 'u' && w[i].mod == M_NONE) {
      i++;
   } else if (i == 1 && i + 1 < n && w[0].base == 'g' && w[i].base == 'i' && w[i].mod == M_NONE && is_vowel(&w[i + 1])) {
      i++;
   }
   if (!in_list(initials, w, 0, i)) return 0;
   *vs = i;
   while (i < n && is_vowel(&w[i])) i++;
   *ve = i;
   if (*ve - *vs > 3) return 0;
   if (*ve == *vs && i < n) return 0;
   return in_list(finals, w, i, n);
}

static int tone_position(const vch_t *w, int n, int vs, int ve, int newstyle) {
   int j, len = ve - vs;

   for (j = ve - 1; j >= vs; j--) {
      if (w[j].mod != M_NONE) return j;
   }
   if (len == 1) return vs;
   if (ve < n) return ve - 1;
   if (len == 3) return vs + 1;
   if (newstyle && ((w[vs].base == 'o' && (w[vs + 1].base == 'a' || w[vs + 1].base == 'e')) ||
                    (w[vs].base == 'u' && w[vs + 1].base == 'y'))) return vs + 1;
   return vs;
}

static int find_last(const vch_t *w, int vs, int ve, unsigned char base) {
   int j;

   for (j = ve - 1; j >= vs; j--) {
      if (w[j].base == base) return j;
   }
   return -1;
}

int ime_key(ime_t *im, const unsigned short *word, int n, int key, unsigned short *out, int *nout) {
   vch_t w[IME_MAXWORD + 2];
   int i, j, vs, ve, tone = 0, tk = 0, kl, up, undo = 0, append = 0;
   int telex = im->mode == IME_TELEX;

   if (im->mode == IME_OFF) return 0;
   if (n == 0) im->suppress = 0;
   if (im->suppress || n > IME_MAXWORD) return 0;
   up = key >= 'A' && key <= 'Z';
   kl = up ? key + 32 : key;
   if (!(kl >= 'a' && kl <= 'z') && !(!telex && key >= '0' && key <= '9')) return 0;

   for (i = 0; i < n; i++) {
      if (!decompose(word[i], &w[i])) return 0;
      if (w[i].tone) tone = w[i].tone;
      w[i].tone = 0;
   }
   if (!parse(w, n, &vs, &ve)) return 0;

   if (telex) {
      tk = kl == 's' ? 1 : kl == 'f' ? 2 : kl == 'r' ? 3 : kl == 'x' ? 4 : kl == 'j' ? 5 : 0;
   } else if (key >= '1' && key <= '5') {
      tk = key - '0';
   }

   if (tk) {
      if (ve == vs) return 0;
      if (tone == tk) {
         tone = 0;
         undo = 1;
      } else {
         tone = tk;
      }
   } else if ((telex && kl == 'z') || (!telex && key == '0')) {
      if (!tone) return 0;
      tone = 0;
   } else if ((telex && (kl == 'a' || kl == 'e' || kl == 'o')) || (!telex && key == '6')) {
      if (telex) {
         j = find_last(w, vs, ve, (unsigned char) kl);
      } else {
         for (j = ve - 1; j >= vs && w[j].base != 'a' && w[j].base != 'e' && w[j].base != 'o'; j--);
         if (j < vs) j = -1;
      }
      if (j < 0) {
         if (!telex) return 0;
         append = 1;
      } else if (w[j].mod == M_HAT) {
         w[j].mod = M_NONE;
         undo = 1;
      } else {
         w[j].mod = M_HAT;
      }
   } else if (!telex && key == '8') {
      j = find_last(w, vs, ve, 'a');
      if (j < 0) return 0;
      if (w[j].mod == M_BREVE) {
         w[j].mod = M_NONE;
         undo = 1;
      } else {
         w[j].mod = M_BREVE;
      }
   } else if ((telex && kl == 'w') || (!telex && key == '7')) {
      for (j = vs; j + 1 < ve && !(w[j].base == 'u' && w[j + 1].base == 'o'); j++);
      if (j + 1 < ve) {
         // the "uo" cluster takes the horn on both letters: ươ
         if (w[j].mod == M_HORN && w[j + 1].mod == M_HORN) {
            w[j].mod = w[j + 1].mod = M_NONE;
            undo = 1;
         } else if (ve - vs == 2 && ve == n && (vs == 1 ? w[0].base == 'h' : vs == 2 && w[0].base == 't' && w[1].base == 'h')) {
            w[j + 1].mod = M_HORN; // thuở, huơ
         } else {
            w[j].mod = w[j + 1].mod = M_HORN;
         }
      } else {
         j = find_last(w, vs, ve, 'u');
         if (j < 0 && telex) j = find_last(w, vs, ve, 'a');
         if (j < 0) j = find_last(w, vs, ve, 'o');
         if (j >= 0) {
            if (w[j].mod == (w[j].base == 'a' ? M_BREVE : M_HORN)) {
               w[j].mod = M_NONE;
               undo = 1;
            } else {
               w[j].mod = w[j].base == 'a' ? M_BREVE : M_HORN;
            }
         } else if (telex && im->w_alone && ve == vs && ve == n) {
            w[n].base = 'u'; // a lone w is ư
            w[n].mod = M_HORN;
            w[n].tone = 0;
            w[n].up = (unsigned char) up;
            n++;
         } else {
            if (!telex) return 0;
            append = 1;
         }
      }
   } else if ((telex && kl == 'd') || (!telex && key == '9')) {
      if (n > 0 && w[0].base == 'd' && vs > 0) {
         if (w[0].mod == M_BAR) {
            w[0].mod = M_NONE;
            undo = 1;
         } else {
            w[0].mod = M_BAR;
         }
      } else {
         if (!telex) return 0;
         append = 1;
      }
   } else {
      append = 1;
   }

   if (append) {
      if (key >= '0' && key <= '9') return 0;
      w[n].base = (unsigned char) kl;
      w[n].mod = M_NONE;
      w[n].tone = 0;
      w[n].up = (unsigned char) up;
      n++;
   }
   if (!parse(w, n, &vs, &ve)) {
      if (append || !undo) return 0;
      ve = vs; // an undo keeps the letters even if they no longer spell a syllable
   }
   if (tone && ve > vs) w[tone_position(w, n, vs, ve, im->newstyle)].tone = (unsigned char) tone;

   for (i = 0; i < n; i++) out[i] = compose(&w[i]);
   if (undo) {
      out[n++] = (unsigned short) key;
      im->suppress = 1;
   }
   *nout = n;
   return 1;
}
