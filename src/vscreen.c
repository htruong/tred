#include <dos.h>
#include <malloc.h>
#include <string.h>
#include "vscreen.h"
#include "screen.h"

virtual_screen *vscreen_init(unsigned char w, unsigned char h) {
    virtual_screen *vs = (virtual_screen *)malloc(sizeof(virtual_screen));
    if (!vs) return NULL;
    vs->width = w;
    vs->height = h;
    vs->cur_x = 0;
    vs->cur_y = 0;
    vs->buffer = (unsigned short *)malloc(sizeof(unsigned short) * w * h);
    if (!vs->buffer) {
        free(vs);
        return NULL;
    }
    memset(vs->buffer, 0, sizeof(unsigned short) * w * h);
    return vs;
}

void vscreen_free(virtual_screen *vs) {
    if (vs) {
        if (vs->buffer) free(vs->buffer);
        free(vs);
    }
}

void vscreen_clear(virtual_screen *vs) {
    if (!vs) return;
    memset(vs->buffer, 0, sizeof(unsigned short) * vs->width * vs->height);
}

void vscreen_set_cursor(virtual_screen *vs, unsigned char x, unsigned char y) {
    if (!vs) return;
    if (x < vs->width) vs->cur_x = x;
    if (y < vs->height) vs->cur_y = y;
}

unsigned short vscreen_puts(virtual_screen *vs, const unsigned short *str, unsigned short len) {
    unsigned char x = vs->cur_x;
    unsigned char y = vs->cur_y;
    unsigned short i = 0;

    if (!vs || !str) return 0;
    for (i = 0; i < len; ++i) {
        unsigned short ch = str[i];
        if (ch == '\r') {
            continue; // Ignore carriage return
        }
        if (ch == '\n') {
            x = 0;
            y++;
            if (y >= vs->height) break;
            continue;
        }
        if (y >= vs->height) break;
        vs->buffer[y * vs->width + x] = ch;
        // Check if we're at the bottom right after writing
        if (x == vs->width - 1 && y == vs->height - 1) {
            i++; // Count this last character as written
            break;
        }
        x++;
        if (x >= vs->width) {
            x = 0;
            y++;
        }
    }
    vs->cur_x = x;
    vs->cur_y = y;
    return i;
}

void vscreen_flush(virtual_screen *vs) {
    if (!vs) return;
    // Dump the whole buffer to the screen at (0,0)
    puttexte(vs->buffer, vs->width * vs->height);
}