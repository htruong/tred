#ifndef VGA_H
#define VGA_H

// Colour indexes 5 and 6 are repainted (see setup_vietnamese_font)
#define COLOR_WHITE    5
#define COLOR_DARKGRAY 6

#define SCAN_LINES_350 0x01
#define SCAN_LINES_400 0x02

void __far * get_fontdef_ptr(unsigned char font_block);
void install_fontdef(unsigned char font_block, unsigned char font_height, unsigned short num_chars, unsigned char start_char, void __far * ptr);
void set_char(unsigned char font_height, unsigned char fontblock, unsigned char ascii_char, void __far * ptr);
void activate_fontblock(unsigned char font_block);
void disable_color_plane_3(void);
void set_scan_lines(unsigned char setting);
void set_palette(unsigned char index, unsigned char value);
void setup_vietnamese_font(int win2k_workaround);
void restore_text_mode(void);
void vga_cursor(int x, int y);
void vga_cursor_shape(unsigned char start, unsigned char end);
#define CURSOR_HIDDEN 0x20
#define CURSOR_INSERT 12
#define CURSOR_OVERWRITE 7
#define CURSOR_END 13

#endif // VGA_H
