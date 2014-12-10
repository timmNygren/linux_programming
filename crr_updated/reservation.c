#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "search_sort_utils.h"
#include "reservation.h"

char RES_ERROR_STR[BUFF] = "";	// REQ6
int res_lookup_size = 0;

#define ERROR_RES( fp, ...) res_error( fp, __FUNCTION__, __LINE__, __VA_ARGS__ "" )		// REQ6

void res_error( FILE* fp, const char* functionname, int lineno, const char* op )	// REQ6
{
	char errbuff[BUFF];
	char* errstr = strerror_r( errno, errbuff, BUFF );
	if( strlen(op) )
	{
		fprintf( fp, "%s:%d (%s): %s\n", functionname, lineno, op, errstr );
	} else {
		fprintf( fp, "%s:%d %s\n", functionname, lineno, errstr );
	}
}

time_t to_local( time_t t )		// REQ11
{
	struct tm utc;
	localtime_r( &t, &utc );
	t += utc.tm_gmtoff;
	return t;
}

time_t to_utc( time_t t )		// REQ11
{
	struct tm localzone;
	localtime_r( &t, &localzone );
	t -= localzone.tm_gmtoff;
	return t;
}

reservation create_reservation( const char* roomname, const time_t start, const time_t end, const char* desc )
{
	time_t tstart, tend;
	tstart = to_utc( start );	// REQ11
	tend = to_utc( end );		// REQ11
	reservation newreservation;
	strncpy( newreservation.roomname, roomname, sizeof( newreservation.roomname ) );
	newreservation.starttime = tstart;
	newreservation.endtime = tend;
	strncpy( newreservation.description, desc, sizeof( newreservation.description ) );
	return newreservation;
}

reservation* update_reservation( reservation* oldreservation, const char* newroomname, const time_t newstart, const time_t newend, const char* newdesc )
{
	time_t tstart, tend;
	tstart = to_utc( newstart );	// REQ11
	tend = to_utc( newend );		// REQ11
	strncpy( oldreservation->roomname, newroomname, sizeof( oldreservation->roomname ) );
	oldreservation->starttime = tstart;
	oldreservation->endtime = tend;
	strncpy( oldreservation->description, newdesc, sizeof( oldreservation->description ) );
	return oldreservation;
}

void res_print_reservation( reservation* res )
{
	char buff1[BUFF];
	char buff2[BUFF];

	time_t t1 = to_local( res->starttime );	// REQ11
	time_t t2 = to_local( res->endtime );	// REQ11

	ctime_r( &t1, buff1 );
	ctime_r( &t2, buff2 );

	buff1[strlen(buff1) - 1] = '\0';
	buff2[strlen(buff2) - 1] = '\0';

	printf( "The %s is reserved from: %s to: %s.\n", res->roomname, buff1, buff2 );
	printf( "\tDescription of the event: %s\n", res->description );
}

void resVect_init( resVect* v )
{
	v->data = NULL;
	v->size = 0;
	v->count = 0;
}

int resVect_count( resVect* v )
{
	return v->count;
}

reservation* resVect_add( resVect* v, reservation res )
{

	reservation* check = bsearch( &res, v->data, v->count, sizeof(reservation), bsearch_conflict );	// REQ5, REQ7

	if( check )
		return check;

	// Add non-conflict reservation
	if( v->size == 0 )
	{
		v->size = 5;
		v->data = calloc( v->size, sizeof(reservation) );	// REQ4
		if( !(v->data) )	// REQ6
		{
			ERROR_RES( stderr, "Error allocating memory adding a reservation" );
			snprintf( RES_ERROR_STR, BUFF, "Error adding a reservation. Quitting the program." );
			exit(1);
		}
	}

	if( v->size == v->count )
	{
		v->size *= 2;
		v->data = realloc( v->data, sizeof(reservation) * v->size );	// REQ4
		if( !(v->data) )	// REQ6
		{
			ERROR_RES( stderr, "Error allocating memory adding a reservation" );
			snprintf( RES_ERROR_STR, BUFF, "Error adding a reservation. Quitting the program." );
			exit(1);
		}
	}

	v->data[v->count] = res;
	v->count++;

	qsort( v->data, v->count, sizeof(reservation), sort_name_time );	// REQ5
	return NULL;
}

void resVect_set( resVect* v, int index, reservation res )
{
	if( index >= v->count || index < 0 )	// REQ6
	{
		fprintf( stderr, "%s:%d: index out of bounds with index %i.\n", __FUNCTION__, __LINE__, index );
		snprintf( RES_ERROR_STR, BUFF, "Error inserting a reservation. Quitting the program." );
		exit(1);
	}
	v->data[index] = res;
}

reservation* resVect_get( resVect* v, int index )
{
	if( index >= v->count || index < 0 )	// REQ6
	{
		fprintf( stderr, "%s:%d: index out of bounds with index %i.\n", __FUNCTION__, __LINE__, index );
		snprintf( RES_ERROR_STR, BUFF, "Error retrieving a reservation. Quitting the program." );
		exit(1);
	}
	reservation* temp = &v->data[index];
	return temp;
}

void resVect_delete( resVect* v, int index )
{
	if( index >= v->count || index < 0 )	// REQ6
	{
		fprintf( stderr, "%s:%d: index out of bounds with index %i.\n", __FUNCTION__, __LINE__, index );
		snprintf( RES_ERROR_STR, BUFF, "Error deleting a reservation. Quitting the program." );
		exit(1);
	}
	for( int i = index; i < v->count - 1; i++ )
	{
		v->data[i] = v->data[i+1];
	}
	v->count--;
}

void resVect_free( resVect* v )	// REQ4
{
	if( v->data )
		free( v->data );
}

void resVect_write_file( resVect* v, char* filename )	// REQ10
{
	FILE* fp;
	fp = fopen( filename, "w" );
	fwrite( v->data, sizeof(reservation), v->count, fp );
	fclose( fp );
}

void resVect_read_file( resVect* v, char* filename )	// REQ3b
{
	FILE* fp;
	if( (fp = fopen( filename, "r")) == NULL )	// REQ6
	{
		fprintf( stderr, "Cannot open file: %s for reading reservations. One may be created.\n", filename );
		return;
	}

	if( fseek( fp, 0, SEEK_END ) != 0 )	// REQ6
	{
		ERROR_RES( stderr, "index out of bounds in fseek" );
		snprintf( RES_ERROR_STR, BUFF, "Error reading reservations. Quitting the program." );
		exit(1);
	}

	long int fcount = ftell( fp );
	rewind( fp );

	v->count = fcount / sizeof(reservation);
	v->size = 5;
	while( v->count >= v->size )
		v->size *= 2;

	v->data = calloc( v->size, sizeof(reservation) );	// REQ4

	if( !(v->data) )	// REQ6
	{
		fputs( "Error allocating memory reading reservations from file.", stderr );
		snprintf( RES_ERROR_STR, BUFF, "Error reading reservations. Quitting the program." );
		exit(1);
	}

	if( fread( v->data, sizeof(reservation), v->count, fp ) != v->count )	// REQ3a
	{
		ERROR_RES( stderr, "Short read of data: fread");
		snprintf( RES_ERROR_STR, BUFF, "Error reading reservations. Quitting the program." );
		exit(1);

	}

	qsort( v->data, v->count, sizeof(reservation), sort_name_time );	// REQ5
	fclose( fp );
}

void resVect_check_consistency( resVect* v, char** rooms, int numrooms )	// REQ8
{
	char** checkname;

	for( int i = 0; i < v->count; i++ )
	{
		checkname = bsearch( v->data[i].roomname, rooms, numrooms, sizeof(char*), bsearch_room_cmp );	// REQ5, REQ8

		if( checkname == NULL )		// REQ6
		{
			fprintf( stderr, "%s:%d: File incosistency. %s is missing from rooms.dat\n", __FUNCTION__, __LINE__, v->data[i].roomname );
			snprintf( RES_ERROR_STR, BUFF, "Inconsistent data in the reservation file. Quitting the program." );
			exit(1);		
		}
	}
}

size_t* resVect_select_room_at_time( resVect* v, time_t key, char** rooms, int numrooms )
{
	qsort( v->data, v->count, sizeof(reservation), sort_time_name );	// REQ5
	time_t timekey = to_utc( key );


	int index = 0;
	size_t reservedrooms[numrooms];
	reservation* reserved;
	reserved = bsearch( &timekey, v->data, v->count, sizeof(reservation), bsearch_time_cmp );	// REQ5

	size_t room_address;
	size_t res_address;
	size_t* available = NULL;
	if( reserved != NULL ) {
		char** foundroom = bsearch( reserved->roomname, rooms, numrooms, sizeof(char*), bsearch_room_cmp );	// REQ5
		room_address = foundroom - rooms;
		res_address = reserved - v->data;
		reservedrooms[index++] = room_address;

		/*
		 * Bsearch is used on each room reservation that contains the given time key. This is to determine
		 * the address in the rooms array to appropriately show the available rooms at the end.
		 */

		// Go left
		size_t lefti = res_address - 1;
		while( lefti >= 0 && timekey >= v->data[lefti].starttime && timekey <= v->data[lefti].endtime )
		{
			foundroom = bsearch( v->data[lefti].roomname, rooms, numrooms, sizeof(char*), bsearch_room_cmp );	// REQ5
			room_address = foundroom - rooms;
			reservedrooms[index++] = room_address;			
			lefti--;
		}

		// Go right
		size_t righti = res_address + 1;
		while( righti < v->count && timekey >= v->data[righti].starttime && timekey <= v->data[righti].endtime )
		{
			foundroom = bsearch( v->data[righti].roomname, rooms, numrooms, sizeof(char*), bsearch_room_cmp );	// REQ5
			room_address = foundroom - rooms;
			reservedrooms[index++] = room_address;
			righti++;
		}

		qsort( reservedrooms, index, sizeof(size_t), sort_int );	// REQ5
		available = calloc( (numrooms - index), sizeof(size_t) );	// REQ4
		if( !available )	// REQ6
		{
			fputs( "Error allocating memory to return available rooms.", stderr );
			snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations. Quitting the program." );
			exit(1);
		}

		int avail_index = 0;
		for( size_t i = 0; i < numrooms; i++ )
		{
			if( bsearch( &i, reservedrooms, index, sizeof(size_t), sort_size_t ) == NULL )	// REQ5
				available[avail_index++] = i;

		}
		res_lookup_size = avail_index;
	}

	return available;
}

size_t* resVect_select_res_day( resVect* v, time_t key )
{
	qsort( v->data, v->count, sizeof(reservation), sort_time_name );	// REQ5

	int day_count = 0;
	int day_size = 5;
	size_t* res_on_day = NULL;
	reservation* res = NULL;

	res = bsearch( &key, v->data, v->count, sizeof(reservation), bsearch_day_cmp );	// REQ5

	size_t res_index;
	if( res ) {
		res_on_day = calloc( day_size, sizeof(size_t) );	// REQ4
		if( !res_on_day )	// REQ6
		{
			fputs( "Error allocating memory to return reservations on a particular day.", stderr );
			snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations on a particular day. Quitting the program." );
			exit(1);
		}
		res_index = res - v->data;
		res_on_day[day_count++] = res_index;

		// Go left
		time_t res_t;
		struct tm res_tm;
		struct tm day_key_tm;
		localtime_r( &key, &day_key_tm );	// REQ11
		size_t lefti = res_index - 1;
		while( lefti >= 0 )
		{
			if( res_index == 0 )	// Forgot size_t can't be negative, this prevents -1 indexing when getting reservations
				break;

			res_t = v->data[lefti].starttime;
			res_t = to_local( res_t );
			localtime_r( &res_t, &res_tm );

			if( res_tm.tm_wday != day_key_tm.tm_wday )
				break;

			if( day_count == day_size )
			{
				day_size *= 2;
				res_on_day = realloc( res_on_day, sizeof(size_t) * day_size );	// REQ4
				if( !res_on_day )	// REQ6
				{
					fputs( "Error reallocating memory to return reservations on a particular day.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations on a particular day. Quitting the program." );
					exit(1);	
				}
			}

			res_on_day[day_count++] = lefti;
			if( lefti == 0 )	// Forgot size_t can't be negative, this prevents -1 indexing when getting reservations
				break;
			else		
				lefti--;
		}

		// Go right
		size_t righti = res_index + 1;
		while( righti < v->count )
		{
			res_t = v->data[righti].starttime;
			res_t = to_local( res_t );
			localtime_r( &res_t, &res_tm );

			if( res_tm.tm_wday != day_key_tm.tm_wday )
				break;

			if( day_count == day_size )
			{
				day_size *= 2;
				res_on_day = realloc( res_on_day, sizeof(size_t) * day_size );	// REQ4
				if( !res_on_day )	// REQ6
				{
					fputs( "Error reallocating memory to return reservations on a particular day.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations on a particular day. Quitting the program." );
					exit(1);	
				}
			}

			res_on_day[day_count++] = righti;
			righti++;
		}

		qsort( res_on_day, day_count, sizeof(size_t), sort_int );	// REQ5

	}

	res_lookup_size = day_count;
	return res_on_day;
}

size_t* resVect_select_res_room( resVect* v, char* key )
{
	qsort( v->data, v->count, sizeof(reservation), sort_name_time );	// REQ5

	time_t timeNow = time( NULL );
	int resCount = 0;
	int resSize = 5;
	size_t* resRooms = NULL;
	reservation* res = NULL;

	res = bsearch( key, v->data, v->count, sizeof(reservation), bsearch_res_room_cmp );	// REQ5

	size_t resIndex;
	if( res )
	{
		resRooms = calloc( resSize, sizeof(size_t) );	// REQ4
		if( !resRooms )		// REQ6
		{
			fputs( "Error allocating memory to return reservations for a particular room.", stderr );
			snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations for a particular room. Quitting the program." );
			exit(1);
		}

		resIndex = res - v->data;

		// Bsearch may find a reservation in the past.
		// We must account for that and search for the first reservation that is occurring or will occur
		// in the near future
		while( resIndex < v->count && timeNow >= v->data[resIndex].endtime )
		{
			resIndex++;
		}

		// Check to make sure we still have a reservation with a room that the user is looking for
		if( strcasecmp( key, v->data[resIndex].roomname ) != 0 )
		{
			return NULL;
		}

		resRooms[resCount++] = resIndex;
		
		// Go left
		size_t lefti = resIndex - 1;
		while( lefti >= 0 )
		{
			if( resIndex == 0 )	// Forgot size_t can't be negative, this prevents -1 indexing when getting reservations
				break;

			if( strcasecmp( key, v->data[lefti].roomname ) != 0 || timeNow >= v->data[lefti].endtime )
			{
				break;
			}
			if( resCount == resSize )
			{
				resSize *= 2;
				resRooms = realloc( resRooms, sizeof(size_t) * resSize );	// REQ4
				if( !resRooms )	// REQ6
				{
					fputs( "Error reallocating memory to return reservations for a particular room.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations for a particular room. Quitting the program." );
					exit(1);	
				}
			}

			resRooms[resCount++] = lefti;	
			if( lefti == 0 )	// Forgot size_t can't be negative, this prevents -1 indexing when getting reservations
				break;
			else	
				lefti--;
		}

		// Go right
		size_t righti = resIndex + 1;
		while( righti < v->count )
		{
			if( strcasecmp( key, v->data[righti].roomname ) != 0 )
			{
				break;
			}
			if( resCount == resSize )
			{
				resSize *= 2;
				resRooms = realloc( resRooms, sizeof(size_t) * resSize );	// REQ4
				if( !resRooms )	// REQ6
				{
					fputs( "Error reallocating memory to return reservations for a particular room.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations for a particular room. Quitting the program." );
					exit(1);	
				}
			}

			resRooms[resCount++] = righti;
			righti++;
		}

		qsort( resRooms, resCount, sizeof(size_t), sort_int );	// REQ5
	}

	res_lookup_size = resCount;
	return resRooms;
}

size_t* resVect_select_res_desc( resVect* v, char* key )
{
	int resCount = 0;
	int resSize = 0;
	size_t* resRooms = NULL;
	for( size_t i = 0; i < v->count; i++)
	{
		if( strcasestr( v->data[i].description, key ) )
		{
			if( resSize == 0 )
			{
				resSize = 5;
				resRooms = calloc( resSize, sizeof(size_t) );	// REQ4
				if( !resRooms )	// REQ6
				{
					fputs( "Error allocating memory to return reservations for a word search in the description.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations for a word search in the description. Quitting the program." );
					exit(1);
				}
			}

			if( resCount == resSize )
			{
				resSize *= 2;
				resRooms = realloc( resRooms, sizeof(size_t) * resSize );	// REQ4
				if( !resRooms )	// REQ6
				{
					fputs( "Error reallocating memory to return reservations for a particular room.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations for a particular room. Quitting the program." );
					exit(1);	
				}
			}
			resRooms[resCount++] = i;
		}
	}
	res_lookup_size = resCount;
	qsort( resRooms, resCount, sizeof(size_t), sort_int );	// REQ5

	return resRooms;
}
