#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "reservation.h"
#include "search_sort_utils.h"
#include "crr_utils.h"

// REQ3c MAIN_MENU
const char* MAIN_MENU[] = { "What would you like to do today?\n", "1. Create a reservation at a particular time.\n", \
			 "2. Search all the rooms for one day.\n", "3. Search for one room over all days.\n", \
			 "4. Search the reservations description for a particular reservation.\n", \
			 "Press enter to quit.\n" };

void main_menu( void )	// REQ3c
{
	for( int i = 0; i < sizeof(MAIN_MENU)/sizeof(char*); i++ )
	{
		fputs( MAIN_MENU[i], stdout );
	}	
}

void print_format_list( void )	// REQ3c
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
}

void crr_print_menu( char** menu, size_t* lookups, int lookups_size, int printNums )	// REQ3c
{
	for( int i = 0; i < lookups_size; i++)
	{
		if( printNums )
			printf( "%d: %s\n", i+1, menu[lookups[i]] );
		else
			printf( "%s\n", menu[lookups[i]] );
	}
}

void print_rooms( char** roomnames, int numRooms, int printNums )	// REQ3c
{
	int i = 0;
	size_t view[numRooms];
	for( size_t i = 0; i < numRooms; i++ )
		view[i] = i;

	crr_print_menu( roomnames, view, numRooms, printNums );
}

time_t get_start_time( void )	// REQ3c
{
	char buf[BUFFLEN];
	int err;
	time_t currentTime = time( NULL );
	time_t startTime;
	struct tm tempTM;

	puts( "\nEnter a start date:" );
	while( fgets( buf, BUFFLEN, stdin ) )
	{
		buf[strlen(buf) - 1] = '\0';
		err = getdate_r( buf, &tempTM );
		if( err == 7 || err ==8 )
		{
			puts( "Invalid format" );
			print_format_list();
			puts( "\nEnter a start date:" );
			continue;
		} else if( err != 0 ) {		// REQ6
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/. Please source datemsk.sh\n", __FUNCTION__, __LINE__, buf, err ); 
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
			continue;
		}
		break;
	}
	return startTime;
}

time_t get_end_time( void )		// REQ3c
{
	char buf[BUFFLEN];
	int err;
	time_t endTime;
	struct tm tempTM;
	puts( "\nEnter an end date:" );
	while( fgets( buf, BUFFLEN, stdin ) )
	{
		buf[strlen(buf) - 1] = '\0';
		err = getdate_r( buf, &tempTM );
		if( err == 7 || err ==8 )
		{
			puts( "Invalid format" );
			print_format_list();
			puts( "\nEnter an end date:" );
			// fflush( stdout );
			continue;
		} else if( err != 0 ) {		// REQ6
			fprintf( stderr, "%s:%d: Error processing %s, with error code /%d/. Please source datemsk.sh\n", __FUNCTION__, __LINE__, buf, err ); 
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

char* get_desc( void )	// REQ3c
{
	char* buf = calloc( DESC_SIZE, sizeof(char) );	// REQ4
	puts( "\nEnter a short description (Limit 128 characters):" );
	fgets( buf, DESC_SIZE, stdin );
	buf[strlen(buf) - 1] = '\0';
	return buf;
}

reservation new_reservation( char* roomname )
{
	char* desc;
	time_t startTime = 0;
	time_t endTime = -1;

	while( startTime > endTime )
	{

		startTime = get_start_time();
		endTime = get_end_time();

		if ( startTime > endTime )	// REQ3c
		{
			puts( "\nThe ending time must come after the starting time." );
			continue;
		}
		
	}

	desc = get_desc();

	reservation temp = create_reservation( roomname, startTime, endTime, desc );
	free( desc );	// REQ4
	return temp;
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
			puts( "\nThe ending time must come after the starting time." );	// REQ3c
			continue;
		}
	}
	desc = get_desc();
	reservation res = create_reservation( roomname, startTime, endTime, desc );
	qsort( v->data, v->count, sizeof(reservation), sort_name_time );	// REQ5
	check = bsearch( &res, v->data, v->count, sizeof(reservation), bsearch_conflict );	// REQ5, REQ7

	if( check != NULL && check != resVect_get( v, res_pos ) )	// REQ7
		return check;


	reservation* updateres = resVect_get( v, res_pos );
	update_reservation( updateres, roomname, startTime, endTime, desc );
	free( desc );	// REQ4
	return NULL;
}

void crr_print_reservations( resVect* v, size_t* lookups, int lookups_size )	// REQ3c
{
	for( int i = 0; i < lookups_size; i++ )
	{
		printf( "%i. ", i+1 );
		res_print_reservation( resVect_get( v, lookups[i] ) );
	}
}
