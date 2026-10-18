#ifndef MOUSE_H
#define MOUSE_H

// Mouse through the DOS mouse driver (INT 33h), in text cells

typedef struct {
   int x, y;              // cell under the pointer
   unsigned char buttons; // bit 0 left, bit 1 right, held now
   unsigned char pressed, released; // the left button went down / up
   signed char wheel;     // wheel notches, + is towards the user
} mouse_ev_t;

int mouse_init(void);
void mouse_done(void);
void mouse_show(void);
void mouse_hide(void);
// Reads what happened since the last call into ev[] (at most 3); returns the count
int mouse_poll(mouse_ev_t *ev);

#endif // MOUSE_H
