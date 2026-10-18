#ifndef IME_H
#define IME_H

// Vietnamese input method (Unikey-style Telex and VNI)

#define IME_OFF   0
#define IME_TELEX 1
#define IME_VNI   2
#define IME_MODES 3

#define IME_MAXWORD 10

typedef struct {
   unsigned char mode;
   unsigned char newstyle; // 1: hoà/thuỷ, 0: hòa/thủy
   unsigned char w_alone;  // Telex: w with no vowel to change types ư
   unsigned char suppress; // an undo happened in this word: type keys literally
} ime_t;

void ime_init(ime_t *im);
// The cursor moved or a non-word key was typed: start a fresh word.
void ime_reset(ime_t *im);
// word: the n code points of the current word left of the cursor. Returns 1
// and the full replacement word in out/nout (at most IME_MAXWORD + 1 code
// points), or 0 when the key should simply be inserted.
int ime_key(ime_t *im, const unsigned short *word, int n, int key, unsigned short *out, int *nout);
// Whether code point cp can be part of a word the IME may rewrite.
int ime_word_char(const ime_t *im, unsigned cp);
const char *ime_name(int mode);

#endif // IME_H
