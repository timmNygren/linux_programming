/***
 * charcell utilities for several in lecture code examples.
 */

#include <ncurses.h>
#include <string.h>
#include <ctype.h>

#include "charcell-utils.h"

void cursestring_init( curseString* string )
{
	string->index = 0;
	string->cstring[string->index] = '\0';
}

void cursestring_add_char( curseString* string, char c )
{
	if( string->index == STRLEN - 1 )
	{
		string->index--;
	}
	string->cstring[string->index++] = c;
	string->cstring[string->index] = '\0';
}

void cursestring_delete_char( curseString* string )
{
	if( string->index == 0 )
		return;
	string->index--;
	string->cstring[string->index] = '\0';
}

void cursestring_get_string( curseString* string, char* dest )
{
	strncpy( dest, string->cstring, STRLEN );
}

void print_save_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 2;
	y = 2;
	char* saveMenu[] = { "Yes", "No" };
	int nChoices = sizeof(saveMenu) / sizeof(char*);

	mvwprintw(menu_win, 1, 2, "Would you like to save? Use arrow keys to go up and down, Press enter to select a choice");
	for(i = 0; i < nChoices; ++i)
	{
		clear_line(menu_win, y);	
		if(highlight == i + 1) /* High light the present choice */
		{	
			wattron(menu_win, A_REVERSE); 
			mvwprintw(menu_win, y, x, "%s", saveMenu[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s", saveMenu[i]);
		++y;
	}
	wrefresh(menu_win);
}

void get_string_input( WINDOW* edit_win, char* dest )
{
	curseString input;
	cursestring_init( &input );
	cursestring_get_string( &input, dest );
	int ch;
	char c;
	while( (ch = getch()) != 10 )
	{
		switch(ch) {
			case KEY_RESIZE:
				break;
			case KEY_BACKSPACE:
				cursestring_delete_char( &input );
				break;
			default:
				if ( isprint(ch) ) {
					// snprintf( dest, BUFLEN, "%c", ch );
					c = (char)ch;
					cursestring_add_char( &input, c );
				}
				break;
		}
		cursestring_get_string( &input, dest );
		clear_line( edit_win, 1 );
		mvwprintw( edit_win, 1, 2, dest );
		wrefresh(edit_win);
	}
	clear_line( edit_win, 1 );
}

void draw_borders(WINDOW* screen, char horiz, char vert, char corner)
{
	int x, y, i;

	getmaxyx(screen, y, x);

	// 4 corners
	mvwaddch(screen, 0, 0, corner);
	mvwaddch(screen, y - 1, 0, corner);
	mvwaddch(screen, 0, x - 1, corner);
	mvwaddch(screen, y - 1, x - 1, corner);

	// sides
	for(i = 1; i < (y - 1); i++) {
		mvwaddch(screen, i, 0, vert);
		mvwaddch(screen, i, x - 1, vert);
	}

	// top and bottom
	for(i = 1; i < (x - 1); i++) {
		mvwaddch(screen, 0, i, horiz);
		mvwaddch(screen, y - 1, i, horiz);
	}
}

int size_display( WINDOW* display, WINDOW* edit )
{
	int parent_x, parent_y;
	int edit_size = 3;

	// always do this at startup or resize, otherwise you have strange initial
	// refresh semantics
	wclear(stdscr);
	refresh();

	// set up initial windows
	getmaxyx(stdscr, parent_y, parent_x);

	wresize(display, parent_y - edit_size, parent_x);
	mvwin(display, 0, 0);
	wclear(display);
	draw_borders(display, HORZ1, VERT1, CORNER);
	mvwprintw(display, 0, 3, DISPLAY_TITLE);
	mvwprintw(display, 0, parent_x-3-strlen(QUIT_TITLE), QUIT_TITLE);

	wresize(edit, edit_size, parent_x);
	mvwin(edit, parent_y-edit_size, 0);
	wclear(edit);
	draw_borders(edit, HORZ2, VERT2, CORNER);
	mvwprintw(edit, 0, 3, EDIT_TITLE);

	wrefresh(display);
	wrefresh(edit);

	return parent_y - 4 - edit_size;
}

void clear_line( WINDOW* screen, int row )
{
	int x, y, i;
	getmaxyx( screen, y, x );

	for(i = 1; i < (x - 2); i++) {
		mvwaddch( screen, row, i, ' ' );
	}
}

