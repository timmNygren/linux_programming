/**
 * Project 1 CRR application using ncurses library
 *
 * Author: Timm Nygren
 * 
 *
 */
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#include "charcell-utils.h"

#include "reservation.h"
#include "crr_utils.h"
#include "search_sort_utils.h"

// #define BUFFLEN 1024
// #define ROOM_NAME_LEN 49
// #define DAY_SEARCH 0
// #define ROOM_SEARCH 1
// #define DESC_SEARCH 2

int fileChanges = 0;
char* reservationfilename;
char** rooms;
int numRooms = 0;
resVect resList;


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

#define ERROR_CRR( fp, ...) crr_error( fp, __FUNCTION__, __LINE__, __VA_ARGS__ "" )

void crr_error( FILE* fp, const char* functionname, int lineno, const char* op )
{
	char errbuff[BUFFLEN];
	char* errstr = strerror_r( errno, errbuff, BUFFLEN );
	if( strlen(op) )
	{
		fprintf( fp, "%s:%d (%s): %s\n", functionname, lineno, op, errstr );
	} else {
		fprintf( fp, "%s:%d %s\n", functionname, lineno, errstr );
	}
}

int compare_string( const void* left, const void* right )
{
	const char* mleft = *(const char**)left;
	const char* mright = *(const char**)right;
	return strcmp( mleft, mright );
}

void cleanup( void )
{
	if( rooms )
	{
		for( int i = 0; i < numRooms; i++ )
		{
			if( rooms[i] )
				free( rooms[i] );
		}
		free( rooms );
	}
	resVect_free( &resList );

	if( strcmp( RES_ERROR_STR, "" ) != 0 )
	{
		// clean up from ncurses
		endwin();
		puts( RES_ERROR_STR );
		puts( "Press enter to continue. . ." );
		getchar();
	}
}

void setup_rooms( char* roomfilename )
{
	FILE* roomsfile;
	if( (roomsfile = fopen( roomfilename, "r" )) == NULL )
	{
		ERROR_CRR( stderr, "open file rooms.dat" );
		puts( "Missing rooms.dat file. Please make sure you have a file with this exact name. Exiting the program." );
		exit(1);
	}

	char buf[BUFF];
	while( fgets(buf, BUFF, roomsfile) && !feof(roomsfile) )
	{
		buf[strlen(buf) - 1] = '\0';
		if( strcmp( buf, "" ) != 0 )
		{
			numRooms++;
		}
	}

	if( numRooms == 0 )
	{
		ERROR_CRR( stderr, "No rooms in file" );
		puts( "There are no rooms available. Exiting the program." );
		exit(1);
	}

	rewind( roomsfile );

	rooms = calloc( numRooms, sizeof(char*) );
	if( !rooms )
	{
		ERROR_CRR( stderr, "allocating memory for rooms" );
		puts( "Something went wrong loading the rooms. Exiting the program." );
		exit(1);
	}

	int i = 0;
	while( fgets( buf, BUFF, roomsfile ) && !feof(roomsfile) )
	{
		buf[strlen(buf) - 1] = '\0';
		if( strcmp( buf, "" ) != 0 && i < numRooms)
		{
			rooms[i] = calloc( ROOM_NAME_LEN, sizeof(char) );
			if( !rooms[i] )
			{
				ERROR_CRR( stderr, "allocating memory for rooms" );
				puts( "Something went wrong loading the rooms. Exiting the program." );
				exit(1);
			}
			strncpy( rooms[i], buf, ROOM_NAME_LEN );
			i++;
		}
	}
	fclose( roomsfile );

	qsort( rooms, numRooms, sizeof(char*), compare_string );
}

void add_reservations( void )
{

	struct tm test;
	int result;
	time_t tstart, tend;
	char* desc1 = "A birthday party with pool.";
	char* desc2 = "Chillin' in the lounge.";
/////
	result = getdate_r("2014/12/02 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Tuesday", result ); 
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/12/03 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Wednesday", result ); 
		exit(1);
	}
	tend = mktime( &test );

	for( int i = 0; i < numRooms - 4; i+= 2 )
	{
		resVect_add( &resList, create_reservation( rooms[i], tstart, tend, "Birthday partay" ) );
	}
// End first

/////
	result = getdate_r("2014/12/01 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Wednesday", result ); 
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/12/02 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Thursday", result ); 
		exit(1);
	}
	tend = mktime( &test );
	if( resVect_add( &resList, create_reservation( rooms[1], tstart, tend, desc1 ) ))
	{
		puts( "Conflicting reservations" );
	}
// End second

/////
	result = getdate_r("2014/12/28 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Friday", result );
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/12/29 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Saturday", result ); 
		exit(1);
	}
	tend = mktime( &test );
	resVect_add( &resList, create_reservation( rooms[1], tstart, tend, desc1 ) );
// end

/////
	result = getdate_r("2014/12/27 at 04PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Thursday", result );
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/12/28 at 04PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Friday", result ); 
		exit(1);
	}
	tend = mktime( &test );
	resVect_add( &resList, create_reservation( rooms[8], tstart, tend, desc2 ) );
// End Third

/////
	char* desc3 = "Doing massive studing for finals";
	result = getdate_r("2014/12/02 at 04PM", &test);
	if( result != 0 )
	{
		printf( "Error processing %s, with error code /%d/\n", "Tuesday", result );
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/12/03 at 04PM", &test);
	if( result != 0 )
	{
		printf( "Error processing %s, with error code /%d/\n", "Wednesday", result ); 
		exit(1);
	}
	tend = mktime( &test );
	resVect_add( &resList, create_reservation( rooms[numRooms - 1], tstart, tend, desc3 ) );
// End fourth
}

// main function
void setup_reservation( void )
{
	char buff[BUFFLEN];
	char searchbuff[64];
	struct tm brokendate;
	int result;
	time_t timekey;
	puts( "Input a date and time to check. Press enter to go back." );
	while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	{
		buff[strlen(buff)-1] = '\0';
		result = getdate_r( buff, &brokendate );
		if( result == 7 || result == 8 )
		{
			puts( "Invalid date. The following list contains valid inputs." );
			print_format_list();
			puts( "\nEnter a date and time to check or press enter to go back." );
		} else if( result != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
			puts( "Error converting date. Exiting program." );
			puts( "Press enter to quit. . ." );
			getchar();
			exit(1);
		}
		timekey = mktime( &brokendate );

		size_t* roomlookups = resVect_select_room_at_time( &resList, timekey, rooms, numRooms );

		printf( "The following rooms are available on %s.\n", buff );
		strncpy( searchbuff, buff, 64 );
		if( roomlookups )
		{
			crr_print_menu( rooms, roomlookups, res_lookup_size, 1 );
		} else {
			print_rooms( rooms, numRooms, 1 );
			res_lookup_size = numRooms;
		}
		puts( "Press enter to go back." );
		int room;
		while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
		{
			if( strlen(buff) > 2 )
			{
				puts( "Not a valid input" );
				continue;
			}

			int err = sscanf(buff, "%d", &room);
			if( err != 1 || room < 1 || room > res_lookup_size )
			{
				puts( "Invalid room id.\n" );
				puts( "The following rooms are available" );
				crr_print_menu( rooms, roomlookups, res_lookup_size, 1 );
				puts( "Press enter to go back." );
				continue;
			}
			room--;	// Make 0 offset
			char* roomname;
			if( roomlookups )
				roomname = rooms[ roomlookups[room] ];

			else
				roomname = rooms[room];

			reservation res = new_reservation( roomname );
			reservation* conflict = resVect_add( &resList, res );
			if( conflict )
			{
				puts( "There was a conflicting reservation:" );
				res_print_reservation( conflict );
				printf( "The following rooms are available on %s.\n", searchbuff );
				crr_print_menu( rooms, roomlookups, res_lookup_size, 1 );
				puts( "Press enter to go back." );
				continue;
			} else {
				fileChanges = 1;
				puts( "Your reservation has been added!" );
				break;
			}
		}

		if( roomlookups )
			free( roomlookups );
		break;
	}
}
#define SELECT( function ) function( &resList, key )
// main function
void review_update_or_delete( size_t* roomlookups ) //int search_type, char** rooms, int numrooms )
{
	char buff[BUFFLEN];
	if( roomlookups ) 
	{
		int choice = 0;

		crr_print_reservations( &resList, roomlookups, res_lookup_size );
		puts( "\nPick a reservation." );
		while( fgets( buff, BUFFLEN, stdin ) )
		{
			int err = sscanf( buff, "%d", &choice );
			if( err != 1 || choice < 1 || choice > res_lookup_size )
			{
				puts( "Invalid choice." );
				crr_print_reservations( &resList, roomlookups, res_lookup_size );
				puts( "\nPick a reservation." );
				continue;
			}
			break;
		}
		choice--;
		printf( "Your choice was %d\n", choice );
		int update;
		puts( "Would you like to update or delete?" );
		puts( "1. Update" );
		puts( "2. Delete" );

		while( fgets( buff, BUFFLEN, stdin ) )
		{
			if( strlen(buff) > 2 )
			{
				puts( "Invalid choice" );
				puts( "Would you like to update or delete?" );
				puts( "1. Update\n2. Delete" );	
				continue;			
			}
			int err = sscanf( buff, "%d", &update );
			if( err != 1 || update < 1 || update > 2 )
			{
				puts( "Invalid choice" );
				puts( "Would you like to update or delete?" );
				puts( "1. Update\n2. Delete" );	
				continue;
			}
			break;

		}

		int room;
		if( update == 1 )
		{
			puts( "Pick a room:" );
			print_rooms( rooms, numRooms, 1 );
			while( fgets( buff, BUFFLEN, stdin ) )
			{
				int err = sscanf(buff, "%d", &room);
				if( err != 1 || room < 1 || room > numRooms )
				{
					puts( "Invalid room id." );
					puts( "The following rooms are:" );
					print_rooms( rooms, numRooms, 1 );
					continue;
				}
				room--;	// Make 0 offset
				break;
			}

			reservation* conflict = crr_update_reservation( rooms[room], &resList, roomlookups[choice] );

			if( conflict )
			{
				puts( "There was a conflicting reservation:" );
				res_print_reservation( conflict );
			} else {
				fileChanges = 1;
				puts( "Your reservation has been updated!" );
			}
		} else {
			resVect_delete( &resList, roomlookups[choice] );
			fileChanges = 1;
			puts( "The reservation was deleted." );
		}
	} else
		puts( "There were no reservations found" );
}

void day_search( void )
{
	char buff[BUFFLEN];
	size_t* roomlookups = NULL;
	struct tm brokendate;
	int result;
	time_t key;
	puts( "Enter a day of the week to check reservation. Press enter to go back." );
	
	while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	{
		buff[strlen(buff)-1] = '\0';

		result = getdate_r( buff, &brokendate );
		if( result == 7 || result == 8 )
		{
			puts( "Invalid input." );
			// puts( "Enter a day of the week to check reservations. Press enter to go back." );
			puts( "Enter a day of the week to check reservation. Press enter to go back." );
			continue;
		} else if( result != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
			puts( "Error converting date. Exiting program." );
			puts( "Press enter to quit. . ." );
			getchar();
			exit(1);
		}

		key = mktime( &brokendate );
		roomlookups = SELECT( resVect_select_res_day );
		break;
	}

	review_update_or_delete( roomlookups );
	if( roomlookups )
		free( roomlookups );
	// qsort( resList.data, resList.count, sizeof(reservation), sort_name_time );
}

void room_search( void )
{
	char buff[ROOM_NAME_LEN];
	char* key = NULL;
	size_t* roomlookups = NULL;
	char** roomCheck = NULL;

	puts( "Enter a room to check reservations over all days. Press enter to go back." );
	print_rooms( rooms, numRooms, 0 );
	while( fgets( buff, ROOM_NAME_LEN, stdin ) && buff[0] != '\n' )
	{
		buff[strlen(buff)-1] = '\0';
		roomCheck = bsearch( buff, rooms, numRooms, sizeof(char*), bsearch_room_cmp );
		if( roomCheck )
		{
			break;
		}
		puts( "Invalid room." );
		puts( "Enter a room to check reservations over all days. Press enter to go back." );
		print_rooms( rooms, numRooms, 0 );
	}
	printf( "Room chosen was %s\n", rooms[roomCheck - rooms] );
	key = calloc( ROOM_NAME_LEN, sizeof(char) );
	if( !key )
	{
		fputs( "Could not allocate memory for key in room search.", stderr );
		puts( "Something went wrong searching the given room. Exiting the program." );
		exit(1);
	}
	strncpy( key, buff, ROOM_NAME_LEN );

	roomlookups = SELECT( resVect_select_res_room );


	// for( int i = 0; i < res_lookup_size; i++ )
	// {
	// 	printf( "Reservation indicies are %li\n", roomlookups[i] );
	// 	// crr_print_reservations( &resList, roomlookups, res_lookup_size );
	// }
	review_update_or_delete( roomlookups );

	if( key )
		free( key );
	if( roomlookups )
		free( roomlookups );
}

void desc_search( void )
{
	char buff[DESC_SIZE];
	char* key = NULL;
	size_t* roomlookups = NULL;

	puts( "Enter a word to search reservation descriptions. Press enter to go back." );
	// print_rooms( rooms, numRooms, 0 );

	fgets( buff, DESC_SIZE, stdin );

	if( buff[0] == '\n' )
		return;

	buff[strlen(buff)-1] = '\0';
		
	key = calloc( DESC_SIZE, sizeof(char) );
	if( !key )
	{
		fputs( "Could not allocate memory for key in description search.", stderr );
		puts( "Something went wrong searching the reservation descriptions. Exiting the program." );
		exit(1);
	}
	strncpy( key, buff, DESC_SIZE );

	roomlookups = SELECT( resVect_select_res_desc );


	// for( int i = 0; i < res_lookup_size; i++ )
	// {
	// 	printf( "Reservation indicies are %li\n", roomlookups[i] );
	// 	// crr_print_reservations( &resList, roomlookups, res_lookup_size );
	// }

	review_update_or_delete( roomlookups );

	if( key )
		free( key );
	if( roomlookups )
		free( roomlookups );
}

void init( int argc, char* argv[] )
{
	if( argc < 2 || argc > 3 )
	{
		puts( "Usage: ./crr rooms.dat [schedule.dat]" );
		puts( "You must provide a file called 'rooms.dat' and must not be empty." );
		puts( "The file 'schedule.dat' is optional. If nothing is provided, schedule.dat will be used for the file name." );
		exit(1);
	} else if ( argc == 2 ) {
		reservationfilename = "schedule.dat";
	} else {
		reservationfilename = argv[2];
	}
	atexit( cleanup );
	setup_rooms( argv[1] );
	resVect_init( &resList );
	// add_reservations();

	resVect_read_file( &resList, reservationfilename );
	resVect_check_consistency( &resList, rooms, numRooms );

	// init ncurses interface
	// initscr();
	// noecho();
	// cbreak();
	// keypad(stdscr,TRUE);		// For arrows and enter/backspace for menu navigation and input
	// curs_set(FALSE);

	// install_handler( SIGHUP );
	// install_handler( SIGUSR1 );
}

void clear_input_buffer( void )
{
	int c;
	do {
		c = getchar();
	} while( c != '\n' && c != EOF );
}

int main( int argc, char* argv[] )
{
	init( argc, argv );
	


	WINDOW* display = newwin(1, 1, 0, 0 );
	WINDOW* edit = newwin(1,1, 0, 0 );
	int dispheight = size_display( display, edit );
	char buf[BUFFLEN];
	int nChoices = 5;
	int highlight = 1;
	int choice = 0;
	int ch;
	while( ch = getch() ) {
		switch (ch) {
			case KEY_RESIZE:
				dispheight = size_display( display, edit );
				// d = 0;
				// strncpy( buf, "KEY_RESIZE", BUFLEN );
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				if( sighup_received ) {
					// snprintf( buf, BUFLEN, "Received SIGHUP %lu", (unsigned long)time(NULL) );
					// mvwprintw( display, d++ + 2, 2, buf );
					// d = d % dispheight;
					sighup_received = 0;
				}
				if( sigusr1_received ) {
					// snprintf( buf, BUFLEN, "Received SIGUSR1 %lu", (unsigned long)time(NULL) );
					// mvwprintw( display, d++ + 2, 2, buf );
					// d = d % dispheight;
					sigusr1_received = 0;
				}
				// wrefresh(display);
				break;
			case KEY_UP:
				if( highlight == 1 )
					highlight = nChoices;
				else
					--highlight;
				strncpy( buf, "KEY_UP", BUFFLEN );
				mvwprintw( edit, 1, 2, buf );

				wrefresh(edit);
				break;
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				break;
			case KEY_DOWN:
				if( highlight == nChoices )
					highlight = 1;
				else 
					++highlight;
				strncpy( buf, "KEY_DOWN", BUFFLEN );
				mvwprintw( edit, 1, 2, buf );

				wrefresh(edit);
				break;
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				// wrefresh(display);
				// break;
			case 10:
				choice = highlight;
				strncpy( buf, "KEY_ENTER", BUFFLEN );
				mvwprintw( edit, 1, 2, buf );
				wrefresh(edit);
				break;
				// mvwprintw( display, d++ + 2, 2, buf );
				// d = d % dispheight;
				// wrefresh(display);
				// break;
		}

		switch( choice ) {
			case 1:
				// puts( "You chose setup_reservation" );
				mvwprintw( edit, 1, 2, "You chose setup_reservation" );
				wrefresh( edit );
				// setup_reservation();
				// size_display( display, edit );
				break;
			case 2:
				// puts( "You chose day_search" );
				mvwprintw( edit, 1, 2, "You chose day_search" );
				wrefresh( edit );
				// day_search();
				// size_display( display, edit );
				break;
			case 3:
				// puts( "You chose room_search" );
				mvwprintw( edit, 1, 2, "You chose room_search" );
				wrefresh( edit );
				// room_search();
				// size_display( display, edit );
				break;
			case 4:
				// puts( "You chose desc_search" );
				mvwprintw( edit, 1, 2, "You chose desc_search" );
				wrefresh( edit );
				// desc_search();
				// size_display( display, edit );
				break;
		}

		// User chose exit or pressed F10
		if( choice == 5 || choice == KEY_F(10) )
		{

			size_display( display, edit ); // Clear both screens
			nChoices = 2;
			highlight = 1;
			choice = 0;

			print_save_menu( display, highlight );
			while( ch = getchar() )
			{
				switch (ch) {
					case KEY_RESIZE:
						dispheight = size_display( display, edit );
						// d = 0;
						// strncpy( buf, "KEY_RESIZE", BUFLEN );
						// mvwprintw( display, d++ + 2, 2, buf );
						// d = d % dispheight;
						if( sighup_received ) {
							// snprintf( buf, BUFLEN, "Received SIGHUP %lu", (unsigned long)time(NULL) );
							// mvwprintw( display, d++ + 2, 2, buf );
							// d = d % dispheight;
							sighup_received = 0;
						}
						if( sigusr1_received ) {
							// snprintf( buf, BUFLEN, "Received SIGUSR1 %lu", (unsigned long)time(NULL) );
							// mvwprintw( display, d++ + 2, 2, buf );
							// d = d % dispheight;
							sigusr1_received = 0;
						}
						// wrefresh(display);
						break;
					case KEY_UP:
						if( highlight == 1 )
							highlight = nChoices;
						else
							--highlight;
						strncpy( buf, "KEY_UP", BUFFLEN );
						mvwprintw( edit, 1, 2, buf );

						wrefresh(edit);
						break;
					case KEY_DOWN:
						if( highlight == nChoices )
							highlight = 1;
						else 
							++highlight;
						strncpy( buf, "KEY_DOWN", BUFFLEN );
						mvwprintw( edit, 1, 2, buf );

						wrefresh(edit);
						break;
					case 10:
						choice = highlight;
						strncpy( buf, "KEY_ENTER", BUFFLEN );
						mvwprintw( edit, 1, 2, buf );

						wrefresh(edit);
						break;
				}

				if( choice == 1 )
				{
					resVect_write_file( &resList, reservationfilename );
					mvwprintw( display, 6, 2, "Reservations saved! Press any key to continue." );
					wrefresh( display );
					getch();
					break;
				} else if( choice == 2 ) {
					mvwprintw( display, 6, 2, "You're reservations will not be saved. Press any key to continue." );
					wrefresh( display );
					getch();
					break;
				}

				print_save_menu( display, highlight );

				// clear_input_buffer();
				// if( c == 'y' || c == 'Y' )
				// {
				// 	if( fileChanges )
				// 	{
				// 		resVect_write_file( &resList, reservationfilename );
				// 		puts( "Reservations saved!" );
				// 	}
				// 	break;
				// } else if( c == 'n' || c == 'N' ) {
				// 	break;
				// }
				// puts( "Please enter Y or N to save." );
			}




			break;
		}	// End choice or F10

		if( choice != 0 )
		{
			highlight = 1;
			choice = 0;
		}
		//main_menu( display, highlight );
		// if( choice != 0 && choice != n_choices )
		// {
		// 	dispheight = size_display( display, edit );
		// 	//clear_line( display, 8 );
		// 	//snprintf( buf, BUFLEN, "You chose choice %d with choice string %s", choice, choices[choice-1] );
		// 	strncpy( buf, "Enter a string", BUFLEN );
		// 	mvwprintw( display, 2, 2, buf );
		// 	wrefresh( display );

		// 	get_string_input( edit, inputstring );

		// 	if( strlen( inputstring ) == 0 )
		// 		strncpy( buf, "You didn't type anything", BUFLEN );
		// 	else
		// 		snprintf( buf, BUFLEN, "You typed string: %s", inputstring );

		// 	mvwprintw( display, 3, 2, buf );
		// 	wrefresh( display );

			
		// 	getch();
		// 	choice = 0;
		// } 
		main_menu( display, highlight );
	}

	// size_display( display, edit ); // Clear both screens
	// nChoices = 2;
	// highlight = 1;
	// choice = 0;
	// int c;
	// mvwprintw( display, 1, 2, "Would you like to save?" );
	// puts( "Would you like to save (Y/N)?" );
	// print_save_menu( display, highlight );
	// while( ch = getchar() )
	// {
	// 	switch (ch) {
	// 		case KEY_RESIZE:
	// 			dispheight = size_display( display, edit );
	// 			d = 0;
	// 			// strncpy( buf, "KEY_RESIZE", BUFLEN );
	// 			// mvwprintw( display, d++ + 2, 2, buf );
	// 			// d = d % dispheight;
	// 			if( sighup_received ) {
	// 				// snprintf( buf, BUFLEN, "Received SIGHUP %lu", (unsigned long)time(NULL) );
	// 				// mvwprintw( display, d++ + 2, 2, buf );
	// 				// d = d % dispheight;
	// 				sighup_received = 0;
	// 			}
	// 			if( sigusr1_received ) {
	// 				// snprintf( buf, BUFLEN, "Received SIGUSR1 %lu", (unsigned long)time(NULL) );
	// 				// mvwprintw( display, d++ + 2, 2, buf );
	// 				// d = d % dispheight;
	// 				sigusr1_received = 0;
	// 			}
	// 			wrefresh(display);
	// 			break;
	// 		case KEY_UP:
	// 			if( highlight == 1 )
	// 				highlight = nChoices;
	// 			else
	// 				--highlight;
	// 			strncpy( buf, "KEY_UP", BUFLEN );
	// 			mvwprintw( edit, 1, 2, buf );

	// 			wrefresh(edit);
	// 			break;
	// 		case KEY_DOWN:
	// 			if( highlight == nChoices )
	// 				highlight = 1;
	// 			else 
	// 				++highlight;
	// 			strncpy( buf, "KEY_DOWN", BUFLEN );
	// 			mvwprintw( edit, 1, 2, buf );

	// 			wrefresh(edit);
	// 			break;
	// 		case 10:
	// 			choice = highlight;
	// 			strncpy( buf, "KEY_ENTER", BUFLEN );
	// 			mvwprintw( edit, 1, 2, buf );

	// 			wrefresh(edit);
	// 			break;
	// 	}

	// 	if( choice == 1 )
	// 	{
	// 		resVect_write_file( &resList, reservationfilename );
	// 		mvwprintw( display, 2, 2, "Reservations saved!" );
	// 		wrefresh( display );
	// 		break;
	// 	} else if( choice == 2 ) {
	// 		mvwprintw( display, 2, 2, "You're reservations will not be saved." );
	// 		wrefresh( display );
	// 		break;
	// 	}

	// 	print_save_menu( display, highlight );

	// 	// clear_input_buffer();
	// 	// if( c == 'y' || c == 'Y' )
	// 	// {
	// 	// 	if( fileChanges )
	// 	// 	{
	// 	// 		resVect_write_file( &resList, reservationfilename );
	// 	// 		puts( "Reservations saved!" );
	// 	// 	}
	// 	// 	break;
	// 	// } else if( c == 'n' || c == 'N' ) {
	// 	// 	break;
	// 	// }
	// 	// puts( "Please enter Y or N to save." );
	// }



	// close curses lib, reset terminal
	endwin();




///////// OLD APPLICATION /////////////
	// puts("\n\n");
	// puts( "Printing all reservations" );
	// for( int i = 0; i < resVect_count( &resList ); i++ )
	// {
	// 	res_print_reservation( resVect_get( &resList, i ) );
	// }
	// puts( "Done printing reservations" );
	// puts("\n\n");

	// puts( "Welcome to Console Room Reservation!\n" );
	// // print_rooms( rooms, numRooms, 1 );
	// // puts( "What would you like to do today?" );
	// main_menu();
	// char buff[BUFFLEN];
	// int choice;
	// while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	// {
	// 	int err = sscanf(buff, "%d", &choice);
	// 	if( err != 1 || choice < 1 || choice > 4 )
	// 	{
	// 		puts( "Invalid choice." );
	// 		main_menu();
	// 		// puts( "The following rooms are:" );
	// 		// print_rooms( rooms, numRooms, 1 );
	// 		continue;
	// 	}
	// 	// room--;	// Make 0 offset
	// 	// break;
	// 	switch( choice ) {
	// 		case 1:
	// 			// puts( "You chose setup_reservation" );
	// 			setup_reservation();
	// 			break;
	// 		case 2:
	// 			// puts( "You chose day_search" );
	// 			day_search();
	// 			break;
	// 		case 3:
	// 			// puts( "You chose room_search" );
	// 			room_search();
	// 			break;
	// 		case 4:
	// 			// puts( "You chose desc_search" );
	// 			desc_search();
	// 			break;
	// 	}
	// 	main_menu();
	// }

	// // setup_reservation();
	// // day_search();
	// // room_search();
	// // desc_search();





	// puts("\n\n");
	// puts( "Printing all reservations" );
	// for( int i = 0; i < resVect_count( &resList ); i++ )
	// {
	// 	res_print_reservation( resVect_get( &resList, i ) );
	// }
	// puts( "Done printing all reservations" );
	// puts("\n\n");

	// int c;
	// puts( "Would you like to save (Y/N)?" );
	// while( c = getchar() )
	// {
	// 	clear_input_buffer();
	// 	if( c == 'y' || c == 'Y' )
	// 	{
	// 		if( fileChanges )
	// 		{
	// 			resVect_write_file( &resList, reservationfilename );
	// 			puts( "Reservations saved!" );
	// 		}
	// 		break;
	// 	} else if( c == 'n' || c == 'N' ) {
	// 		break;
	// 	}
	// 	puts( "Please enter Y or N to save." );
	// }

	return 0;
}















