#ifndef MENU_H
#define MENU_H

#include "editor.h"

// open: index of the menu whose title is highlighted, or -1
void menu_bar_draw(int open);
// Index of the menu whose title is at column x of the bar, or -1
int menu_hit(int x);
// Run the menus starting with menu `which`; returns the chosen CMD_* or CMD_NONE
int menu_run(editor_t *ed, int which);
// Modal message box; returns the button index or -1 for Esc
int msgbox(const char *title, const char * const *lines, const char * const *buttons, int nbtn, int def);
// Modal one-line ASCII input; returns 1 on OK
int inputbox(const char *title, const char *prompt, char *buf, int max);
// File Open dialog with a directory listing; may change the current
// directory. Returns 1 with the chosen file name in name.
int open_dialog(char *name, int max);

#endif // MENU_H
