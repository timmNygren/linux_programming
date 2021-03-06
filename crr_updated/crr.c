/***
 *	CRR application for linux programming
 *
 *	Author: Timm Nygren
 *	
 *	Resources:
 *		http://linux.die.net/man/3/getdate_r		- info for getdate_r and error codes
 *		https://gist.github.com/EmilHernvall/953968 - for a vector like data structure for reservations
 *
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "reservation.h"
#include "crr_utils.h"
#include "search_sort_utils.h"

int fileChanges = 0;	// REQ10
char* reservationfilename;
char** rooms;
int numRooms = 0;
resVect resList;

#define ERROR_CRR( fp, ...) crr_error( fp, __FUNCTION__, __LINE__, __VA_ARGS__ "" )		// REQ6

void crr_error( FILE* fp, const char* functionname, int lineno, const char* op )		// REQ6
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

void cleanup( void )	// REQ4, I feel this is for the dynamic allocation requirement since you must free what you allocate
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
		puts( RES_ERROR_STR );
		puts( "Press enter to continue. . ." );
		getchar();
	}
}

void setup_rooms( char* roomfilename )	// REQ3a, REQ3b
{
	FILE* roomsfile;
	if( (roomsfile = fopen( roomfilename, "r" )) == NULL )	// REQ3a, REQ6
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

	if( numRooms == 0 )		// REQ6
	{
		ERROR_CRR( stderr, "No rooms in file" );
		puts( "There are no rooms available. Exiting the program." );
		exit(1);
	}

	rewind( roomsfile );

	rooms = calloc( numRooms, sizeof(char*) );		// REQ4
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
			rooms[i] = calloc( ROOM_NAME_LEN, sizeof(char) );	// REQ4
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

	qsort( rooms, numRooms, sizeof(char*), compare_string );	// REQ5
}

// Option 1
void setup_reservation( void )	// REQ3c
{
	char buff[BUFFLEN];
	char searchbuff[64];
	struct tm brokendate;
	int result;
	time_t timekey;
	
	puts( "\nInput a date and time to check. Press enter to go back." );
	while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	{
		buff[strlen(buff)-1] = '\0';
		result = getdate_r( buff, &brokendate );
		if( result == 7 || result == 8 )
		{
			puts( "\nInvalid date. The following list contains valid inputs." );
			print_format_list();
			puts( "Enter a date and time to check or press enter to go back." );
			continue;
		} else if( result != 0 ) {	// REQ6
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/. Please source datemsk.sh\n", __FUNCTION__, __LINE__, buff, result ); 
			puts( "Error converting date. Exiting program." );
			puts( "Press enter to quit. . ." );
			getchar();
			exit(1);
		}
		timekey = mktime( &brokendate );

		size_t* roomlookups = resVect_select_room_at_time( &resList, timekey, rooms, numRooms );

		strncpy( searchbuff, buff, 64 );
		printf( "\nThe following rooms are available on %s.\n", buff );
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
			int err = sscanf(buff, "%d", &room);
			if( err != 1 || room < 1 || room > res_lookup_size )
			{
				puts( "\nInvalid room id.\n" );
				printf( "The following rooms are available on %s.\n", searchbuff );
				// crr_print_menu( rooms, roomlookups, res_lookup_size, 1 );
				if( roomlookups )
				{
					crr_print_menu( rooms, roomlookups, res_lookup_size, 1 );
				} else {
					print_rooms( rooms, numRooms, 1 );
					// res_lookup_size = numRooms;
				}
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
			reservation* conflict = resVect_add( &resList, res );	// REQ7
			if( conflict )	// REQ7
			{
				puts( "\nThere was a conflicting reservation:" );
				res_print_reservation( conflict );
				printf( "\nThe following rooms are available on %s.\n", searchbuff );
				if( roomlookups )
					crr_print_menu( rooms, roomlookups, res_lookup_size, 1 );
				else
					print_rooms( rooms, numRooms, 1 );

				puts( "Press enter to go back." );
				continue;
			} else {
				fileChanges = 1;	// REQ10
				puts( "\nYour reservation has been added!\n" );
				break;
			}
		}

		if( roomlookups )
			free( roomlookups );	// REQ4
		break;
	}
}

#define SELECT( function ) function( &resList, key )
// Used in options 2, 3, and 4
void review_update_or_delete( size_t* roomlookups )		// REQ3c
{
	char buff[BUFFLEN];
	if( roomlookups ) 
	{
		int choice = 0;

		puts("\nHere are the reserved rooms.");
		crr_print_reservations( &resList, roomlookups, res_lookup_size );
		puts( "\nPick a reservation. Press enter to go back." );
		while( fgets( buff, BUFFLEN, stdin ) )
		{
			if( buff[0] == '\n' )
				return;
			int err = sscanf( buff, "%d", &choice );
			if( err != 1 || choice < 1 || choice > res_lookup_size )
			{
				puts( "\nInvalid choice. Here are the reserved rooms.\n" );
				crr_print_reservations( &resList, roomlookups, res_lookup_size );
				puts( "\nPick a reservation. Press enter to go back." );
				continue;
			}
			break;
		}
		choice--;	// Make choice base 0

		int update;
		puts( "\nWould you like to update or delete? Press enter to go back." );
		puts( "1. Update\n2. Delete" );

		while( fgets( buff, BUFFLEN, stdin ) )
		{
			if( buff[0] == '\n' )
				return;			
			int err = sscanf( buff, "%d", &update );
			if( err != 1 || update < 1 || update > 2 )
			{
				puts( "\nInvalid choice\n" );
				puts( "Would you like to update or delete? Press enter to go back." );
				puts( "1. Update\n2. Delete" );	
				continue;
			}
			break;

		}

		int room;
		if( update == 1 )
		{
			puts( "\nPick a room:" );
			print_rooms( rooms, numRooms, 1 );
			while( fgets( buff, BUFFLEN, stdin ) )
			{
				int err = sscanf(buff, "%d", &room);
				if( err != 1 || room < 1 || room > numRooms )
				{
					puts( "\nInvalid room id.\n" );
					puts( "The following rooms are:" );
					print_rooms( rooms, numRooms, 1 );
					continue;
				}
				room--;	// Make 0 offset
				break;
			}

			reservation* conflict = crr_update_reservation( rooms[room], &resList, roomlookups[choice] );	// REQ7

			if( conflict )	// REQ7
			{
				puts( "\nThere was a conflicting reservation:" );
				res_print_reservation( conflict );
			} else {
				fileChanges = 1;	// REQ10
				puts( "\nYour reservation has been updated!\n" );
			}
		} else {
			resVect_delete( &resList, roomlookups[choice] );
			fileChanges = 1;	// REQ10
			puts( "\nThe reservation was deleted.\n" );
		} // update == 1
	} else	// roomlookups
		puts( "\nThere were no reservations found\n" );
}

// Option 2
void day_search( void )		// REQ3c
{
	char buff[BUFFLEN];
	size_t* roomlookups = NULL;
	struct tm brokendate;
	int result;
	time_t key;
	puts( "\nEnter a day of the week to check reservation. Press enter to go back." );
	
	while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	{
		buff[strlen(buff)-1] = '\0';

		result = getdate_r( buff, &brokendate );
		if( result == 7 || result == 8 )
		{
			puts( "\nPlease enter a day of the week (Monday, Tuesday...) to check reservation. Press enter to go back." );
			continue;
		} else if( result != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/. Please source datemsk.sh\n", __FUNCTION__, __LINE__, buff, result ); 
			puts( "\nError converting date. Exiting program." );
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
		free( roomlookups );	// REQ4
}

// Option 3
void room_search( void )		// REQ3c
{
	char buff[ROOM_NAME_LEN];
	char* key = NULL;
	size_t* roomlookups = NULL;
	char** roomCheck = NULL;

	puts( "\nHere is a list of valid room names.");
	print_rooms( rooms, numRooms, 0 );
	puts( "\nEnter a room to check reservations over all days. Press enter to go back." );
	while( fgets( buff, ROOM_NAME_LEN, stdin ) )
	{
		if( buff[0] == '\n' )
			return;
		buff[strlen(buff)-1] = '\0';
		roomCheck = bsearch( buff, rooms, numRooms, sizeof(char*), bsearch_room_cmp );	// REQ5
		if( roomCheck )
		{
			break;
		}
		puts( "\nInvalid room. Listing valid room names." );
		print_rooms( rooms, numRooms, 0 );
		puts( "\nEnter a room to check reservations over all days. Press enter to go back." );
	}

	key = calloc( ROOM_NAME_LEN, sizeof(char) );	// REQ4
	if( !key )
	{
		fputs( "Could not allocate memory for key in room search.", stderr );
		puts( "Something went wrong searching the given room. Exiting the program." );
		exit(1);
	}
	strncpy( key, buff, ROOM_NAME_LEN );

	roomlookups = SELECT( resVect_select_res_room );

	review_update_or_delete( roomlookups );

	if( key )	// REQ4
		free( key );
	if( roomlookups )
		free( roomlookups );
}

// Option 4
void desc_search( void )	// REQ3c
{
	char buff[DESC_SIZE];
	char* key = NULL;
	size_t* roomlookups = NULL;

	puts( "\nEnter a word to search reservation descriptions. Press enter to go back." );

	fgets( buff, DESC_SIZE, stdin );

	if( buff[0] == '\n' )
		return;

	buff[strlen(buff)-1] = '\0';
		
	key = calloc( DESC_SIZE, sizeof(char) );	// REQ4
	if( !key )
	{
		fputs( "Could not allocate memory for key in description search.", stderr );
		puts( "Something went wrong searching the reservation descriptions. Exiting the program." );
		exit(1);
	}
	strncpy( key, buff, DESC_SIZE );

	roomlookups = SELECT( resVect_select_res_desc );

	review_update_or_delete( roomlookups );

	if( key )	// REQ4
		free( key );
	if( roomlookups )
		free( roomlookups );
}

void init( int argc, char* argv[] )
{
	if( argc < 2 || argc > 3 )	// REQ3a, REQ3b
	{
		puts( "Usage: ./crr rooms.dat [schedule.dat]" );
		puts( "You must provide a file called 'rooms.dat' and must not be empty." );
		puts( "The file 'schedule.dat' is optional. If nothing is provided, schedule.dat will be used for the file name." );
		exit(1);
	} else if ( argc == 2 ) {
		reservationfilename = "schedule.dat";	// REQ3b
	} else {
		reservationfilename = argv[2];			// REQ3b
	}
	atexit( cleanup );
	setup_rooms( argv[1] );		// REQ3a
	resVect_init( &resList );

	resVect_read_file( &resList, reservationfilename );			// REQ3b
	resVect_check_consistency( &resList, rooms, numRooms );		// REQ8

}

// Clears the input buffer when saving
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

	puts( "Welcome to Console Room Reservation!\n" );	// REQ3c
	main_menu();

	char buff[BUFFLEN];
	int choice;
	while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	{
		int err = sscanf(buff, "%d", &choice);
		if( err != 1 || choice < 1 || choice > 4 )		// REQ6
		{
			puts( "\nInvalid choice.\n" );
			main_menu();
			continue;
		}
		switch( choice ) {
			case 1:
				setup_reservation();
				break;
			case 2:
				day_search();
				break;
			case 3:
				room_search();
				break;
			case 4:
				desc_search();
				break;
		}
		main_menu();
	}

	int c;
	puts( "Would you like to save (Y/N)?" );	// REQ10
	while( c = getchar() )
	{
		clear_input_buffer();
		if( c == 'y' || c == 'Y' )
		{
			puts( "\nReservations saved!\n" );
			if( fileChanges )		// REQ10
			{
				resVect_write_file( &resList, reservationfilename );
			}
			break;
		} else if( c == 'n' || c == 'N' ) {
			puts( "\nReservations were not saved.\n" );
			break;
		}
		puts( "Please enter Y or N to save." );
	}
	puts( "Have a great day!" );
	return 0;
}
