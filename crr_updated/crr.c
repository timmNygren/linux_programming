#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "reservation.h"
#include "crr_utils.h"

// #define BUFFLEN 1024
// #define ROOM_NAME_LEN 49
#define DAY_SEARCH 0
#define ROOM_SEARCH 1
#define DESC_SEARCH 2

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
		resVect_add( &resList, create_reservation( rooms[i], tstart, tend, desc1 ) );
	}
// End first

/////
	result = getdate_r("2014/11/26 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Wednesday", result ); 
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/11/27 at 06PM", &test);
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
	result = getdate_r("2014/11/28 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Friday", result );
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/11/29 at 06PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Saturday", result ); 
		exit(1);
	}
	tend = mktime( &test );
	resVect_add( &resList, create_reservation( rooms[1], tstart, tend, desc1 ) );
// end

/////
	result = getdate_r("2014/11/27 at 04PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Thursday", result );
		exit(1);
	}
	tstart = mktime( &test );
	result = getdate_r("2014/11/28 at 04PM", &test);
	if( result != 0 )
	{
		printf( "%d: Error processing %s, with error code /%d/\n", __LINE__, "Friday", result ); 
		exit(1);
	}
	tend = mktime( &test );
	resVect_add( &resList, create_reservation( rooms[8], tstart, tend, desc2 ) );
// End Third

	// puts("\n\n");
	// for( int i = 0; i < resVect_count( &resList ); i++ )
	// {
	// 	if( resVect_get( &resList, i ) == NULL )
	// 	{
	// 		ERROR( stderr, "Index out of bounds" );
	// 		puts( "Error printing reservations. Quitting the program" );
	// 		exit(1);
	// 	}
	// 	res_print_reservation( resVect_get( &resList, i ) );
	// }

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
			puts( "\nSelect a date and time to check or press enter to go back." );
		} else if( result != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
			puts( "Error converting date. Exiting program." );
			puts( "Press enter to quit. . ." );
			getchar();
			exit(1);
		} else {
			// printf( "Input is: %s\n", buff );
			timekey = mktime( &brokendate );

			size_t* roomlookups = resVect_select_valid_rooms( &resList, timekey, rooms, numRooms );

			puts( "The following rooms are available." );
			if( roomlookups )
			{
				crr_print_menu( rooms, roomlookups, res_lookup_size );
			} else {
				print_rooms( rooms, numRooms );
				res_lookup_size = numRooms;
			}
			puts( "Press enter to go back." );
			int room;
			// char buff[BUFFLEN];
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
					crr_print_menu( rooms, roomlookups, res_lookup_size );
					puts( "Press enter to go back." );
					continue;
				}
				room--;	// Make 0 offset
				// printf( "Your rooms choice was %s\n", rooms[roomlookups[room]] );
				// break;
				// puts( "TESTING REFACTORED NEW_RESERVATION" );
				char* roomname;
				if( roomlookups )
					roomname = rooms[ roomlookups[room] ];
					// reservation res = new_reservation( rooms[ roomlookups[room] ] );
				else
					roomname = rooms[room];
				reservation res = new_reservation( roomname );
				// printf( "ROOM chosen was %s\n", rooms[ roomlookups[room] ] );
				// puts( "TESTING REFACTORED NEW_RESERVATION" );
				// if( strcmp( res.roomname, "" ) == 0 )
				// {
				// 	puts( "The following rooms are available" );
				// 	crr_print_menu( rooms, roomlookups, res_lookup_size );
				// 	puts( "Press enter to go back." );
				// 	continue;
				// } else {
				reservation* conflict = resVect_add( &resList, res );
				if( conflict )
				{
					puts( "There was a conflicting reservation:" );
					res_print_reservation( conflict );
					puts( "The following rooms are available" );
					crr_print_menu( rooms, roomlookups, res_lookup_size );
					puts( "Press enter to go back." );
					continue;
				} else {
					puts( "Your reservation has been added!" );
					break;
				}
				// }
			}

			if( roomlookups )
				free( roomlookups );
			break;
		}
	}

}
#define SELECT( function ) function( &resList, key )
// main function
void review_update_or_delete( size_t* roomlookups ) //int search_type, char** rooms, int numrooms )
{
	// char* message = NULL;

	// char buff[BUFFLEN];
	// struct tm brokendate;
	// int result;
	// time_t key;
	// switch( search_type )
	// {
	// 	case DAY_SEARCH:
	// 		message = "Enter a day of the week to check reservation. Press enter to go back.";
	// 		break;
	// 	case ROOM_SEARCH:
	// 		message = "Enter a room to check reservations. Press enter to go back.";
	// 		break;	
	// }
	// puts( "Enter a day of the week to check reservations. Press enter to go back." );
	// puts( message );
	// while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
	// {
		// buff[strlen(buff)-1] = '\0';
		// result = getdate_r( buff, &brokendate );
		// if( result == 7 || result == 8 )
		// {
		// 	puts( "Invalid input." );
		// 	// puts( "Enter a day of the week to check reservations. Press enter to go back." );
		// 	puts( message );
		// } else if( result != 0 ) {
		// 	fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
		// 	puts( "Error converting date. Exiting program." );
		// 	puts( "Press enter to quit. . ." );
		// 	getchar();
		// 	exit(1);
		// } //else {

		// printf( "Input is: %s\n", buff );
		// key = mktime( &brokendate );

		// size_t* roomlookups = NULL;

		// switch( search_type )
		// {
		// 	case DAY_SEARCH:
		// 		struct tm brokendate;
		// 		int result;
		// 		time_t key;
		// 		// roomlookups = resVect_select_res_day( &resList, key );
		// 		// buff[strlen(buff)-1] = '\0';
		// 		result = getdate_r( buff, &brokendate );
		// 		if( result == 7 || result == 8 )
		// 		{
		// 			puts( "Invalid input." );
		// 			// puts( "Enter a day of the week to check reservations. Press enter to go back." );
		// 			puts( message );
		// 			continue;
		// 		} else if( result != 0 ) {
		// 			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buff, result ); 
		// 			puts( "Error converting date. Exiting program." );
		// 			puts( "Press enter to quit. . ." );
		// 			getchar();
		// 			exit(1);
		// 		}

		// 		// printf( "Input is: %s\n", buff );
		// 		key = mktime( &brokendate );
		// 		roomlookups = SELECT( resVect_select_res_day );
		// 		break;
		// 	case ROOM_SEARCH:

		// 		break;	
		// }
	char buff[BUFFLEN];
	if( roomlookups ) 
	{
		int choice = 0;

		crr_print_rooms( &resList, roomlookups, res_lookup_size );
		puts( "\nPick a reservation." );
		while( fgets( buff, BUFFLEN, stdin ) )
		{
			if( strlen(buff) > 2 )
			{
				puts( "Invalid choice." );	
				crr_print_rooms( &resList, roomlookups, res_lookup_size );
				puts( "\nPick a reservation." );					
			}
			int err = sscanf( buff, "%d", &choice );
			if( err != 1 || choice < 1 || choice > res_lookup_size )
			{
				puts( "Invalid choice." );
				crr_print_rooms( &resList, roomlookups, res_lookup_size );
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
		// printf( "Your choice was int: %d char: %c\n", update, update );

		int room;
		if( update == 1 )
		{
			puts( "Pick a room:" );
			print_rooms( rooms, numRooms );
			while( fgets( buff, BUFFLEN, stdin ) )
			{
				// if( strlen(buff) > 2 )
				// {
				// 	puts( "Not a valid input" );
				// 	continue;
				// }

				int err = sscanf(buff, "%d", &room);
				if( err != 1 || room < 1 || room > numRooms )
				{
					puts( "Invalid room id." );
					puts( "The following rooms are:" );
					print_rooms( rooms, numRooms );
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
				puts( "Your reservation has been updated!" );
			}
		} else {
			resVect_delete( &resList, roomlookups[choice] );
			puts( "The reservation was deleted." );
		}
	} else
		puts( "There were no reservations found" );



		// TODO: Get user choice to edit or delete reservation



		// printf( "Here are the reservations on %s.\n", buff );
		// crr_print_menu( rooms, roomlookups, res_lookup_size );
		// puts( "Press enter to go back." );
		// int room;
		// char buff[BUFFLEN];
		// while( fgets( buff, BUFFLEN, stdin ) && buff[0] != '\n' )
		// {
		// 	if( strlen(buff) > 2 )
		// 	{
		// 		puts( "Not a valid input" );
		// 		continue;
		// 	}

		// 	int err = sscanf(buff, "%d", &room);
		// 	if( err != 1 || room < 1 || room > res_lookup_size )
		// 	{
		// 		puts( "Invalid room id.\n" );
		// 		puts( "The following rooms are available" );
		// 		crr_print_menu( rooms, roomlookups, res_lookup_size );
		// 		puts( "Press enter to go back." );
		// 		continue;
		// 	}
		// 	room--;	// Make 0 offset
		// 	// printf( "Your rooms choice was %s\n", rooms[roomlookups[room]] );
		// 	// break;
		// 	reservation res = new_reservation( rooms[ roomlookups[room] ] );
		// 	if( strcmp( res.roomname, "" ) == 0 )
		// 	{
		// 		puts( "The following rooms are available" );
		// 		crr_print_menu( rooms, roomlookups, res_lookup_size );
		// 		puts( "Press enter to go back." );
		// 		continue;
		// 	} else {
		// 		reservation* conflict = resVect_add( &resList, res );
		// 		if( conflict )
		// 		{
		// 			puts( "There was a conflicting reservation:" );
		// 			res_print_reservation( conflict );
		// 			puts( "The following rooms are available" );
		// 			crr_print_menu( rooms, roomlookups, res_lookup_size );
		// 			puts( "Press enter to go back." );
		// 			continue;
		// 		} else {
		// 			puts( "Your reservation has been added!" );
		// 			break;
		// 		}
		// 	}
		// }

		// if( roomlookups )
		// 	free( roomlookups );
		// break;

		// }
	// }	// while
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

		// roomlookups = resVect_select_res_day( &resList, key );
		// buff[strlen(buff)-1] = '\0';
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

		// printf( "Input is: %s\n", buff );
		key = mktime( &brokendate );
		roomlookups = SELECT( resVect_select_res_day );
		break;
	}

	review_update_or_delete( roomlookups );
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
	add_reservations();

	// resVect_read_file( &resList, reservationfilename );
	// resVect_check_consistency( &resList, rooms, numRooms );


	// puts( "Room List" );
	// for( int i = 0; i < numRooms; i++ )
	// {
	// 	printf( "Room at index %d is %s\n", i, rooms[i] );
	// }
	// puts( "End room list" );

	// init ncurses interface
}

int main( int argc, char* argv[] )
{
	init( argc, argv );
	// char* schedulefile = "schedule.dat";
	
	// resVect_read_file( &resList, reservationfilename );
	// add_reservations();

	puts("\n\n");
	for( int i = 0; i < resVect_count( &resList ); i++ )
	{
		res_print_reservation( resVect_get( &resList, i ) );
	}

	puts("\n\n");
	// setup_reservation();
	day_search();

	puts("\n\n");
	for( int i = 0; i < resVect_count( &resList ); i++ )
	{
		res_print_reservation( resVect_get( &resList, i ) );
	}
	// snprintf( RES_ERROR_STR, BUFF, "Could not open %s", argv[2] );

	// if( strcmp( RES_ERROR_STR, "" ) != 0 )
	// 	printf( "Testing reservation error global string: %s\n", RES_ERROR_STR );

	// TODO: Do not create conflicting reservations

	
	// resVect_write_file( &resList, reservationfilename );

	// if( resVect_delete( &resList, 2 ) == ARRAY_OUT_OF_BOUNDS )
	// {
	// 	ERROR( stderr, "Index out of bounds" );
	// 	puts( "Error deleting a reservation. Quitting the program" );
	// 	exit(1);
	// }
	// // delete first

	// puts("\n\n");
	// for( int i = 0; i < resVect_count( &resList ); i++ )
	// {
	// 	if( resVect_get( &resList, i ) == NULL )
	// 	{
	// 		ERROR( stderr, "Index out of bounds" );
	// 		puts( "Error printing reservations. Quitting the program" );
	// 		exit(1);
	// 	}
	// 	res_print_reservation( resVect_get( &resList, i ) );
	// }





	// struct Reservation resArray[];

	// char buf[BUFFLEN];
	// int choice;
	// print_rooms( rooms, numRooms );
	// puts( "\nWelcome to Room Reservation!\n" );
	// main_menu();
	
	// while( fgets( buf, BUFFLEN, stdin ) && buf[0] != '\n' )
	// {
	// 	puts("");
	// 	if( strlen(buf) > 2 ) 
	// 	{
	// 		fputs( "Not valid input", stdout );
	// 	} else {
	// 		//printf( "Your choice was %c\n", buf[0] );
	// 		choice = buf[0];
	// 		switch( choice )
	// 		{
	// 			case '1':
	// 				puts( "Checking rooms at particular time" );
	// 				fputs( "Enter a time or press enter to go back\n", stdout );
	// 				int result = 0;
	// 				struct tm chosentime;
	// 				time_t chosentime_t;
	// 				while( fgets( buf, BUFFLEN, stdin ) && buf[0] != '\n' )
	// 				{
	// 					// result = getdate_r( buf, &chosentime );
	// 					if( result > 0 && result < 7 )
	// 					{
	// 						fprintf( stderr, "Error processing %s, with error code %d\n", buf, result );
	// 						puts( "Error interpreting your choice. Quitting the program." );
	// 						exit(1);
	// 					} else if( result == 7 || result == 8 ) {
	// 						puts( "Invalid input.\nEnter a time or press enter to go back" );
	// 					} else {

	// 						if( resVect_count( &resList ) == 0 )	// Check using the view array
	// 						{
	// 							print_rooms( rooms, numRooms );
	// 							puts( "Choose a room number:" );
	// 							int room;
	// 							while( fgets( buf, BUFFLEN, stdin ) && buf[0] != '\n' )
	// 							{
	// 								if( strlen(buf) > 2 ) 
	// 								{
	// 									fputs( "Not valid input", stdout );
	// 								} else {
	// 									int err = sscanf( buf, "%d", &room );
	// 									if( err != 1 || room < 1 || room > numRooms )
	// 									{
	// 										puts( "Invalid room id." );
	// 										print_rooms( rooms, numRooms );
	// 										puts( "Choose a room number:" );
	// 									} else {
	// 										resVect_add( &resList, new_reservation( choice ) );
	// 										break;
	// 									}
	// 								}
	// 							}	// End Choose room number while
	// 							break;
	// 						}
	// 					}	// End Enter time while
	// 				}



	// 				// menu for sorting
	// 				// reservation_menu();
	// 				// while( fgets( buf, BUFFLEN, stdin ) && buf[0] != '\n' )
	// 				// {
	// 				// 	if ( strlen(buf) > 2 )
	// 				// 	{
	// 				// 		fputs( "Not valid input", stdout );
	// 				// 		reservation_menu();
	// 				// 	} else {
	// 				// 		choice = buf[0];
	// 				// 		if( choice == '1' || choice == '2' || choice == '3')
	// 				// 		{
	// 				// 			print_reservations( resArray, choice );
	// 				// 			break;
	// 				// 		} else {
	// 				// 			fputs( "Not a valid input", stdout );
	// 				// 			reservation_menu();
	// 				// 		}
	// 				// 	}
	// 				// }
	// 				break;
	// 			case '2':
	// 				puts( "Searching all rooms for one day" );
	// 				// int addedReservation = FALSE;
	// 				// for( int i = 0; i < NUM_RESERVATIONS; i++ )
	// 				// {
	// 				// 	if( resArray[i].roomid == 0 )
	// 				// 	{
	// 				// 		resArray[i] = new_reservation();
	// 				// 		addedReservation = TRUE;
	// 				// 		break;
	// 				// 	}
	// 				// }
	// 				// if ( !addedReservation )
	// 				// {
	// 				// 	fputs( "The reservation list is full.", stdout );
	// 				// }
	// 				// struct Reservation temp = new_reservation();
	// 				break;
	// 			case '3':
	// 				puts( "Searching one room over all days" );
	// 				break;
	// 			case '4':
	// 				puts( "Search for room by description" );
	// 				break;
	// 			case '5':
	// 				puts( "Listing formats" );
	// 				print_format_list();
	// 				break;
	// 			default:
	// 				puts( "Not a valid input" );
	// 				break;
	// 		}	// End switch case
	// 	}
	// 	// puts("TESTING");
	// 	// for( int i = 0; i < resVect_count( &resList ); i++ )
	// 	// {
	// 	// 	res_print_reservation( resVect_get( &resList, i ), rooms );
	// 	// }
	// 	// puts("END TESTING");

	// 	puts("");
	// 	main_menu();
	// }	// End Main while loop

	// cleanup();
	return 0;
}















