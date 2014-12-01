#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "reservation.h"
#include "crr_utils.h"

const char* MAIN_MENU[] = { "What would you like to do today?\n", "1. Create a reservation at a particular time.\n", \
			 "2. Search all the rooms for one day.\n", "3. Search for one room over all days.\n", \
			 "4. Search the reservations description for a particular reservation.\n", \
			 "5. List of valid date formats.\n", "Press enter to quit.\n" };

// const char* RESERVATION_MENU[] = { "How would you like to view the reservations?\n", \
// 			"1. Sorted by Room\n", "2. Sorted by start time\n", "3. Sorted by end time\n", "Press enter to cancle.\n" };

void main_menu( void )
{
	for( int i = 0; i < sizeof(MAIN_MENU)/sizeof(char*); i++ )
	{
		fputs( MAIN_MENU[i], stdout );
	}	
	fflush( stdout );
}

void print_format_list( void )
{
	fputs( "\nList of valid date entries:\n", stdout );
	fputs( "Full weekday name (i.e. Tuesday)\n", stdout );
	fputs( "Weekday at hour(AM/PM) (i.e. Tuesday at 01PM): hours must be in form 01-12\n", stdout );
	fputs( "Weekday at hour:minute(AM/PM) (i.e. Tuesday at 01:30PM)\n", stdout );
	fputs( "Day of Month at hour(AM/PM) (i.e. 10 at 01PM): day of month form 01-31; leading 0 is optional\n", stdout );
	fputs( "Day of Month at hour:minute(AM/PM) (i.e. 10 at 01:30PM)\n", stdout );
	fputs( "YYYY/MM/DD hour(AM/PM) (i.e 2014/11/10 06AM)\n", stdout );
	fputs( "YYYY/MM/DD at hour(AM/PM) (i.e 2014/11/10 at 06AM)\n", stdout );
	fputs( "YYYY/MM/DD hour:minute(AM/PM) (i.e 2014/11/10 06:30AM)\n", stdout );
	fputs( "YYYY/MM/DD at hour:minute(AM/PM) (i.e 2014/11/10 at 06:30AM)\n\n", stdout );
	fflush( stdout );
}

void crr_print_menu( char** menu, size_t* lookups, int lookups_size )
{
	for( int i = 0; i < lookups_size; i++)
	{
		printf( "%d: %s\n", i+1, menu[lookups[i]] );
		// puts( menu[lookups[i]] );
	}
	fflush( stdout );
}

void print_rooms( char** roomnames, int numRooms )
{
	int i = 0;
	size_t view[numRooms];
	for( size_t i = 0; i < numRooms; i++ )
	{
		view[i] = i;
		// printf( "view index %d has value %d\n", i, view[i] );
	}
	crr_print_menu( roomnames, view, numRooms );
}

time_t get_start_time( void )
{
	char buf[BUFFLEN];
	int err;
	time_t currentTime = time( NULL );
	time_t startTime;
	struct tm tempTM;
	puts( "Enter a start date:" );
	fflush( stdout );
	while( fgets( buf, BUFFLEN, stdin ) )
	{
		// fgets( buf, BUFFLEN, stdin );
		buf[strlen(buf) - 1] = '\0';
		err = getdate_r( buf, &tempTM );
		if( err == 7 || err ==8 )
		{
			puts( "Invalid format" );
			print_format_list();
			continue;
		} else if( err != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buf, err ); 
			puts( "Error converting date. Exiting program." );
			puts( "Press enter to quit. . ." );
			getchar();
			exit(1);
		}
		startTime = mktime( &tempTM );
		if( startTime < currentTime )
		{
			puts( "\nYou can't make a reservation in the past!" );
			puts( "Enter a start date:" );
			fflush( stdout );
			continue;
		}
		break;
	}
	return startTime;
}

time_t get_end_time( void )
{
	char buf[BUFFLEN];
	int err;
	time_t endTime;
	struct tm tempTM;
	puts( "Enter an end date:" );
	fflush( stdout );
	while( fgets( buf, BUFFLEN, stdin ) )
	{
		// fgets( buf, BUFFLEN, stdin );
		buf[strlen(buf) - 1] = '\0';
		err = getdate_r( buf, &tempTM );
		if( err == 7 || err ==8 )
		{
			puts( "Invalid format" );
			print_format_list();
			continue;
		} else if( err != 0 ) {
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/\n", __FUNCTION__, __LINE__, buf, err ); 
			puts( "Error converting date. Exiting program." );
			puts( "Press enter to quit. . ." );
			getchar();
			exit(1);
		}
		endTime = mktime( &tempTM );
		break;
	}
	return endTime;
}

char* get_desc( void )
{
	char* buf = calloc( DESC_SIZE, sizeof(char) );
	puts( "Enter a short description (Limit 128 characters):" );
	fflush( stdout );
	fgets( buf, DESC_SIZE, stdin );
	buf[strlen(buf) - 1] = '\0';
	// char* temp = buf;
	return buf;
}

reservation new_reservation( char* roomname )
{
	// struct Reservation temp = {};
	char* desc;
	// int err;
	// int room;
	// struct tm tempTM;
	time_t startTime = 0;
	time_t endTime = -1;
	// puts( "" );
	// print_rooms();
	// puts( "Which room would you like to reserve?" );
	// fflush( stdout );

	// fgets( buf, BUFFLEN, stdin );
	// int err = sscanf(buf, "%d", &room);
	// if( err != 1 || room < 1 || room > 10 )
	// {
	// 	fputs( "Invalid room id. Returning to main menu.", stderr );
	// 	return temp;
	// }
	// time_t  currentTime = time( NULL );
	while( startTime > endTime )
	{

		startTime = get_start_time();
		endTime = get_end_time();

		// printf( "start time is: %li and end time is %li\n", startTime, endTime );
		if ( startTime > endTime )
		{
			puts( "The ending time must come after the starting time" );
			// fputs( "Entered date with starting time > ending time\n", stderr );
			continue;
		}
		
	}

	desc = get_desc();

	reservation temp = create_reservation( roomname, startTime, endTime, desc );
	free( desc );
	return temp;
//	// return create_reservation( roomname, startTime, endTime, desc );
}

reservation* crr_update_reservation( char* roomname, resVect* v, int res_pos )
{

	reservation* check = NULL;
	char* desc;
	time_t startTime = 0;
	time_t endTime = -1;

	while( startTime > endTime )
	{
		startTime = get_start_time();
		endTime = get_end_time();
		if( startTime > endTime )
		{
			puts( "The ending time must come after the starting time." );
			continue;
		}
	}
	desc = get_desc();
	reservation res = create_reservation( roomname, startTime, endTime, desc );
	check = bsearch( &res, v->data, v->count, sizeof(reservation), resVect_bsearch_conflict );

	if( check != NULL && check != resVect_get( v, res_pos ) )
		return check;


	reservation* updateres = resVect_get( v, res_pos );
//	// puts( "\nTESTING UPDATE" );
//	// res_print_reservation( updateres );
	update_reservation( updateres, roomname, startTime, endTime, desc );
//	// res_print_reservation( updateres );
//	// puts( "END TESTING UPDATE\n" );
//	// resVect_set( v, res_pos, res );
	free( desc );
	return NULL;
}

void crr_print_rooms( resVect* v, size_t* lookups, int lookups_size )
{
	for( int i = 0; i < lookups_size; i++ )
	{
		printf( "%i. ", i+1 );
		res_print_reservation( resVect_get( v, lookups[i] ) );
	}
}