#ifndef VGA_H
#define VGA_H

#define SCAN_LINES_350 0x01
#define SCAN_LINES_400 0x02

void __far * get_fontdef_ptr(unsigned char font_block);
void install_fontdef(unsigned char font_block, unsigned char font_height, unsigned short num_chars, unsigned char start_char, void __far * ptr);
void set_char(unsigned char font_height, unsigned char fontblock, unsigned char ascii_char, void __far * ptr);
void activate_fontblock(unsigned char font_block);
void disable_color_plane_3(void);
void set_scan_lines(unsigned char setting);
void setup_vietnamese_font(int win2k_workaround);
void restore_text_mode(void);

#endif // VGA_H
