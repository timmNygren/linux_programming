/***
 * charcell utilities for several in lecture code examples.
 */

#ifndef CHARCELL_UTILS_H
#define CHARCELL_UTILS_H

#define CORNER '+'
#define VERT1  '|'
#define VERT2  ':'
#define HORZ1  '-'
#define HORZ2  '='

#define DISPLAY_TITLE "| Display |"
#define EDIT_TITLE "| Edit |"
#define QUIT_TITLE "| F10->Quit |"

#define STRLEN 129


typedef struct CurseString {
	int index;
	char cstring[STRLEN];
} curseString;

void cursestring_init( curseString* string );
void cursestring_add_char( curseString* string, char c );
void cursestring_delete_char( curseString* string );
void cursestring_get_string( curseString* string, char* dest );

void print_save_menu(WINDOW *menu_win, int highlight);

void get_string_input( WINDOW* edit_win, char* dest );

void draw_borders(WINDOW* screen, char horiz, char vert, char corner);

int size_display( WINDOW* display, WINDOW* edit );

void clear_line( WINDOW* screen, int row );

#endif /* CHARCELL_UTILS_H */
