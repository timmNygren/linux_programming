/***
 * Example code for setting flags indicating signal arrival within the context
 * of a curses application.
 */
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include <ncurses.h>

#include "charcell-utils.h"

int sigusr1_received = 0;
int sighup_received = 0;
void signal_catcher( int signum )
{
	switch( signum ) {
		case SIGHUP : sighup_received = 1; break;
		case SIGUSR1 : sigusr1_received = 1; break;
	}
	/***
	 * The "kick in the pants" trick.  We send ourselves a SIGWINCH 
	 * to make sure that getch() in our primary process thread has a "key"
	 * to return.
	 *
	 * Otherwise we would have to wait around until the window *really* changed
	 * or a key press was sent to the terminal.  Either way would result in a clunky user
	 * interface experience.
	 *
	 * Note that raise(2) IS async signal safe (see signal(7)).
	 */
	raise( SIGWINCH );
}

void install_handler( int signum )
{
	/***
	 * setup a signal handler --- always use sigaction, avoid signal(2)
	 *
	 * another equally valid approach is to setup an independent handler for
	 * each signal.  one handler seems easier to explain in lecture.
	 */
	struct sigaction act;
	memset( &act, 0, sizeof(act) );

	act.sa_handler = signal_catcher;   // set the function to call on signal

	// prevent reentrant signals --- wise to do whether you are using one or 
	// multiple signal handler functions
	sigfillset( &act.sa_mask );

	/***
	 * NOTE:  in order for the SIGWINCH "kick in the pants" trick to work, we 
	 * don't want the ncurses read(2) command to restart.  So DON'T set SA_RESTART
	 * for these signal handlers!
	 */

	if( sigaction( signum, &act, NULL )) {
		perror("sigaction" );
		exit(1);
	}
}

void get_string_input( WINDOW *edit_win, char* dest )
{
	curseString input;
	cursestring_init( &input );

	int ch;
	char c;
	while( ch = getch() != 10 )
	{
		switch(ch) {
			case KEY_RESIZE:
				break;
			case KEY_BACKSPACE:
				curse_delete_char( &input );
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
}

char *choices[] = { "Choice 1", "Choice 2", "Choice 3", "Exit" };
int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 2;
	y = 2;
	//box(menu_win, 0, 0);
	mvwprintw(menu_win, 1, 2, "Use arrow keys to go up and down, Press enter to select a choice");
	for(i = 0; i < n_choices; ++i)
	{
		clear_line(menu_win, y);	
		if(highlight == i + 1) /* High light the present choice */
		{	
			wattron(menu_win, A_REVERSE); 
			mvwprintw(menu_win, y, x, "%s", choices[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s", choices[i]);
		++y;
	}
	wrefresh(menu_win);
}

#define BUFLEN 1024
int main(int argc, char *argv[])
{
	initscr();
	noecho();
	cbreak();
	keypad(stdscr,TRUE);		// For arrows and enter/backspace for menu navigation and input
	curs_set(FALSE);

	install_handler( SIGHUP );
	install_handler( SIGUSR1 );

	// set up initial windows
	WINDOW* display = newwin(1, 1, 0, 0 );
	WINDOW* edit = newwin(1,1, 0, 0 );
	int dispheight = size_display( display, edit );

	int d = 0;
	char buf[BUFLEN];
	int ch;

	int highlight = 1;
	int choice = 0;
	//mvwprintw(display, 1, 2, "Use arrow keys to go up and down, Press enter to select a choice");
	print_menu( display, highlight );
	// wrefresh(display);
	while((ch = getch()) != KEY_F(10)) {
		switch (ch) {
			case KEY_RESIZE:
				dispheight = size_display( display, edit );
				d = 0;
				strncpy( buf, "KEY_RESIZE", BUFLEN );
				mvwprintw( display, d++ + 2, 2, buf );
				d = d % dispheight;
				if( sighup_received ) {
					snprintf( buf, BUFLEN, "Received SIGHUP %lu", (unsigned long)time(NULL) );
					mvwprintw( display, d++ + 2, 2, buf );
					d = d % dispheight;
					sighup_received = 0;
				}
				if( sigusr1_received ) {
					snprintf( buf, BUFLEN, "Received SIGUSR1 %lu", (unsigned long)time(NULL) );
					mvwprintw( display, d++ + 2, 2, buf );
					d = d % dispheight;
					sigusr1_received = 0;
				}
				wrefresh(display);
				break;
			case KEY_UP:
				if(highlight == 1)
					highlight = n_choices;
				else
					--highlight;
				strncpy( buf, "KEY_UP", BUFLEN );
				mvwprintw( edit, 1, 2, buf );

				wrefresh(edit);
				break;
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				break;
			case KEY_DOWN:
				if(highlight == n_choices)
					highlight = 1;
				else 
					++highlight;
				strncpy( buf, "KEY_DOWN", BUFLEN );
				mvwprintw( edit, 1, 2, buf );

				wrefresh(edit);
				break;
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				// wrefresh(display);
				// break;
			case 10:
				choice = highlight;
				strncpy( buf, "KEY_ENTER", BUFLEN );
				mvwprintw( edit, 1, 2, buf );

				wrefresh(edit);
				break;
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				// wrefresh(display);
				// break;
			case KEY_BACKSPACE:
				// choice = highlight;
				strncpy( buf, "KEY_BACKSPACE", BUFLEN );
				mvwprintw( edit, 1, 2, buf );

				wrefresh(edit);
				break;
			default :
				if ( isprint(ch) ) {
					snprintf( buf, BUFLEN, "%c", ch );
					clear_line( edit, 1 );
					mvwprintw( edit, 1, 2, buf );
					wrefresh(edit);
				} else {
					snprintf( buf, BUFLEN, "Unprintable %04x", ch );
					mvwprintw( display, d++ + 2, 2, buf );
					d = d % dispheight;
					wrefresh(display);
				}
				break;
		}
		//print_menu( display, highlight );
		if( choice != 0 && choice != n_choices )
		{
			dispheight = size_display( display, edit );
			//clear_line( display, 8 );
			//snprintf( buf, BUFLEN, "You chose choice %d with choice string %s", choice, choices[choice-1] );
			strncpy( buf, "Enter a string", BUFLEN );
			mvwprintw( display, 2, 2, buf );
			wrefresh( display );

			get_string_input( edit, buf );

			if( strlen( buf ) == 0 )
				strncpy( buf, "You didn't type anything", BUFLEN );
			else
				snprintf( buf, BUFLEN, "You typed string: %s", buf );
			
			mvwprintw( display, 3, 2, buf );
			clear_line( edit, 1 );
			wrefresh( display );

			
			getch();
			choice = 0;
		} 
		print_menu( display, highlight );
	}

	// close curses lib, reset terminal
	endwin();

	return 0;
}

