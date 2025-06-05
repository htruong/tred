#ifndef VSCREEN_H
#define VSCREEN_H

typedef struct {
    unsigned char width;
    unsigned char height;
    unsigned char cur_x;
    unsigned char cur_y;
    unsigned short *buffer; // Each cell is a "character" (cp437e or unicode)
} virtual_screen;

// Initialize a virtual screen of size w x h. Returns pointer to virtual_screen, or NULL on failure.
virtual_screen *vscreen_init(unsigned char w, unsigned char h);

// Free the virtual screen and its buffer.
void vscreen_free(virtual_screen *vs);

// Set the cursor position for the next write.
void vscreen_set_cursor(virtual_screen *vs, unsigned char x, unsigned char y);

// Write a string (unsigned short*, len) at the current cursor position, wrapping as needed.
unsigned short vscreen_puts(virtual_screen *vs, const unsigned short *str, unsigned short len);

// Flush the virtual screen buffer to the real screen at (0,0) using puttexte.
void vscreen_flush(virtual_screen *vs);

void vscreen_clear(virtual_screen *vs);

#endif // VSCREEN_H
