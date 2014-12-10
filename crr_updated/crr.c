#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
		} else if( result != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
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
			reservation* conflict = resVect_add( &resList, res );
			if( conflict )
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
				fileChanges = 1;
				puts( "\nYour reservation has been added!\n" );
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

		puts("\nHere are the rooms reserved on the specified day.");
		crr_print_reservations( &resList, roomlookups, res_lookup_size );
		puts( "\nPick a reservation. Press enter to go back." );
		while( fgets( buff, BUFFLEN, stdin ) )
		{
			if( buff[0] == '\n' )
				return;
			int err = sscanf( buff, "%d", &choice );
			if( err != 1 || choice < 1 || choice > res_lookup_size )
			{
				puts( "\nInvalid choice.\n" );
				crr_print_reservations( &resList, roomlookups, res_lookup_size );
				puts( "\nPick a reservation. Press enter to go back." );
				continue;
			}
			break;
		}
		choice--;
		// printf( "Your choice was %d\n", choice );
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

			reservation* conflict = crr_update_reservation( rooms[room], &resList, roomlookups[choice] );

			if( conflict )
			{
				puts( "\nThere was a conflicting reservation:" );
				res_print_reservation( conflict );
			} else {
				fileChanges = 1;
				puts( "\nYour reservation has been updated!\n" );
			}
		} else {
			resVect_delete( &resList, roomlookups[choice] );
			fileChanges = 1;
			puts( "\nThe reservation was deleted.\n" );
		} // update == 1
	} else	// roomlookups
		puts( "\nThere were no reservations found\n" );
}

void day_search( void )
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
			// puts( "\nInvalid input.\n" );
			// puts( "Enter a day of the week to check reservations. Press enter to go back." );
			puts( "\nPlease enter a day of the week (Monday, Tuesday...) to check reservation. Press enter to go back." );
			continue;
		} else if( result != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
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
		free( roomlookups );
	// qsort( resList.data, resList.count, sizeof(reservation), sort_name_time );
}

void room_search( void )
{
	char buff[ROOM_NAME_LEN];
	char* key = NULL;
	size_t* roomlookups = NULL;
	char** roomCheck = NULL;

	puts( "\nEnter a room to check reservations over all days. Press enter to go back." );
	print_rooms( rooms, numRooms, 0 );
	while( fgets( buff, ROOM_NAME_LEN, stdin ) )
	{
		if( buff[0] == '\n' )
			return;
		buff[strlen(buff)-1] = '\0';
		roomCheck = bsearch( buff, rooms, numRooms, sizeof(char*), bsearch_room_cmp );
		if( roomCheck )
		{
			break;
		}
		puts( "\nInvalid room. Listing room names." );
		print_rooms( rooms, numRooms, 0 );
		puts( "\nEnter a room to check reservations over all days. Press enter to go back." );
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
	

	puts("\n\n");
	puts( "Printing all reservations" );
	for( int i = 0; i < resVect_count( &resList ); i++ )
	{
		res_print_reservation( resVect_get( &resList, i ) );
	}
	puts( "Done printing reservations" );
	puts("\n\n");

	puts( "Welcome to Console Room Reservation!\n" );
	// print_rooms( rooms, numRooms, 1 );
	// puts( "What would you like to do today?" );
	main_menu();
	char buff[BUFFLEN];
	int choice;
	while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	{
		int err = sscanf(buff, "%d", &choice);
		if( err != 1 || choice < 1 || choice > 4 )
		{
			puts( "\nInvalid choice.\n" );
			main_menu();
			// puts( "The following rooms are:" );
			// print_rooms( rooms, numRooms, 1 );
			continue;
		}
		// room--;	// Make 0 offset
		// break;
		switch( choice ) {
			case 1:
				// puts( "You chose setup_reservation" );
				setup_reservation();
				break;
			case 2:
				// puts( "You chose day_search" );
				day_search();
				break;
			case 3:
				// puts( "You chose room_search" );
				room_search();
				break;
			case 4:
				// puts( "You chose desc_search" );
				desc_search();
				break;
		}
		main_menu();
	}

	// setup_reservation();
	// day_search();
	// room_search();
	// desc_search();


	puts("\n\n");
	puts( "Printing all reservations" );
	for( int i = 0; i < resVect_count( &resList ); i++ )
	{
		res_print_reservation( resVect_get( &resList, i ) );
	}
	puts( "Done printing all reservations" );
	puts("\n\n");

	int c;
	puts( "Would you like to save (Y/N)?" );
	while( c = getchar() )
	{
		clear_input_buffer();
		if( c == 'y' || c == 'Y' )
		{
			if( fileChanges )
			{
				resVect_write_file( &resList, reservationfilename );
				puts( "Reservations saved!" );
			}
			break;
		} else if( c == 'n' || c == 'N' ) {
			break;
		}
		puts( "Please enter Y or N to save." );
	}

	return 0;
}















