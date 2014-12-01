#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "reservation.h"

char RES_ERROR_STR[BUFF] = "";
int res_lookup_size = 0;

#define ERROR_RES( fp, ...) res_error( fp, __FUNCTION__, __LINE__, __VA_ARGS__ "" )

void res_error( FILE* fp, const char* functionname, int lineno, const char* op )
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

reservation create_reservation( const char* roomname, const time_t start, const time_t end, const char* desc )
{
	time_t tstart, tend;
	tstart = to_utc( start );
	tend = to_utc( end );
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
	tstart = to_utc( newstart );
	tend = to_utc( newend );
	strncpy( oldreservation->roomname, newroomname, sizeof( oldreservation->roomname ) );
	oldreservation->starttime = tstart;
	oldreservation->endtime = tend;
	strncpy( oldreservation->description, newdesc, sizeof( oldreservation->description ) );
	return oldreservation;
}

void res_print_reservation( reservation* res )
{
	// puts( "Not implemented yet" );
	char buff1[BUFF];
	char buff2[BUFF];

	time_t t1 = to_local( res->starttime );
	time_t t2 = to_local( res->endtime );

	ctime_r( &t1, buff1 );
	ctime_r( &t2, buff2 );

	buff1[strlen(buff1) - 1] = '\0';
	buff2[strlen(buff2) - 1] = '\0';

	printf( "The %s is reserved from: %s to: %s.\n", res->roomname, buff1, buff2 );
	printf( "\tDescription of the event: %s\n", res->description );
}

time_t to_local( time_t t )
{
	struct tm utc;
	localtime_r( &t, &utc );
	t += utc.tm_gmtoff;
	return t;
}

time_t to_utc( time_t t )
{
	struct tm localzone;
	localtime_r( &t, &localzone );
	t -= localzone.tm_gmtoff;
	return t;
}

int res_sort_name_time( const void* left, const void* right )
{
	const reservation *mleft = (const reservation*)left;
	const reservation *mright = (const reservation*)right;
	if( strcmp( mleft->roomname, mright->roomname ) < 0 )
	{
		return -1;
	} else if( strcmp( mleft->roomname, mright->roomname ) > 0 ) {
		return 1;
	} else {
		size_t left_t = mleft->starttime;
		size_t right_t = mright->starttime;
		if( left_t < right_t )
			return -1;
		else if( left_t > right_t )
			return 1;
	} 
	return 0;
}

int res_sort_time_name( const void* left, const void* right )
{
	const reservation *mleft = (const reservation*)left;
	const reservation *mright = (const reservation*)right;
	if( mleft->starttime < mright->starttime ) //&& mleft->endtime < mright->endtime && mleft->endtime <= mright->starttime )
	{
		return -1;
	} else if( mleft->starttime > mright->starttime ) { // && mleft->endtime > mright->endtime && mleft->endtime > mright->starttime ) {
		return 1;
	} else {
		return ( strcmp( mleft->roomname, mright->roomname) );
	}
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

int resVect_bsearch_conflict( const void* key, const void* element )
{
	const reservation* k = (const reservation*)key;
	const reservation* res = (const reservation*)element;

	if( strcmp( k->roomname, res->roomname ) < 0 )
		return -1;
	else if( strcmp( k->roomname, res->roomname ) > 0 )
		return 1;
	else {
		if( k->endtime <= res->starttime )
			return -1;
		else if( k->starttime >= res->endtime )
			return 1;
	}
	return 0;
}

reservation* resVect_add( resVect* v, reservation res )
{
	// check new reservation
	reservation* check = bsearch( &res, v->data, v->count, sizeof(reservation), resVect_bsearch_conflict );

	if( check )
		return check;


	if( v->size == 0 )
	{
		v->size = 5;
		v->data = calloc( v->size, sizeof(reservation) );
		if( !(v->data) )
		{
			ERROR_RES( stderr, "Error allocating memory adding a reservation" );
			// puts( "Error adding a reservation. Quitting the program." );
			snprintf( RES_ERROR_STR, BUFF, "Error adding a reservation. Quitting the program." );
			exit(1);
		}
	}

	if( v->size == v->count )
	{
		v->size *= 2;
		v->data = realloc( v->data, sizeof(reservation) * v->size );
		if( !(v->data) )
		{
			ERROR_RES( stderr, "Error allocating memory adding a reservation" );
			// puts( "Error adding a reservation. Quitting the program." );
			snprintf( RES_ERROR_STR, BUFF, "Error adding a reservation. Quitting the program." );
			exit(1);
		}
	}

	v->data[v->count] = res;
	v->count++;

	qsort( v->data, v->count, sizeof(reservation), res_sort_name_time );
	return check;
}

void resVect_set( resVect* v, int index, reservation res )
{
	if( index >= v->count || index < 0 )
	{
		// ERROR_RES( stderr, "index out of bounds" );
		fprintf( stderr, "%s:%d: index out of bounds\n", __FUNCTION__, __LINE__ );
		// puts( "Error deleting a reservation. Quitting the program" );
		snprintf( RES_ERROR_STR, BUFF, "Error inserting a reservation. Quitting the program." );
		exit(1);
	}
	v->data[index] = res;
}

reservation* resVect_get( resVect* v, int index )
{
	if( index >= v->count || index < 0 )
	{
		// ERROR_RES( stderr, "index out of bounds" );
		fprintf( stderr, "%s:%d: index out of bounds\n", __FUNCTION__, __LINE__ );
		// puts( "Error deleting a reservation. Quitting the program" );	
		snprintf( RES_ERROR_STR, BUFF, "Error retrieving a reservation. Quitting the program." );
		exit(1);
	}
	reservation* temp = &v->data[index];
	return temp;
}

void resVect_delete( resVect* v, int index )
{
	if( index >= v->count || index < 0 )
	{
		// ERROR_RES( stderr, "index out of bounds" );
		fprintf( stderr, "%s:%d: index out of bounds\n", __FUNCTION__, __LINE__ );
		// puts( "Error deleting a reservation. Quitting the program" );
		snprintf( RES_ERROR_STR, BUFF, "Error deleting a reservation. Quitting the program." );
		// RES_ERROR_STR = "Error deleting a reservation. Quitting the program";
		exit(1);
	}
	for( int i = index; i < v->count - 1; i++ )
	{
		v->data[i] = v->data[i+1];
	}
	v->count--;
}

void resVect_free( resVect* v )
{
	if( v->data )
		free( v->data );
}

void resVect_write_file( resVect* v, char* filename )
{
	FILE* fp;
	fp = fopen( filename, "w" );
	fwrite( v->data, sizeof(reservation), v->count, fp );
	// printf( "Writing to file %s with count %d\n", filename, v->count );
	fclose( fp );
}

void resVect_read_file( resVect* v, char* filename )
{
	FILE* fp;
	if( (fp = fopen( filename, "r")) == NULL )
	{
		fprintf( stderr, "Cannot open file: %s for reading reservations. One may be created.\n", filename );
		return;
	}

	if( fseek( fp, 0, SEEK_END ) != 0 )
	{
		ERROR_RES( stderr, "index out of bounds" );
		snprintf( RES_ERROR_STR, BUFF, "Error reading reservations. Quitting the program." );
		// puts( "Error reading reservations. Quitting the program" );
		exit(1);
	}

	long int fcount = ftell( fp );
	rewind( fp );

	v->count = fcount / sizeof(reservation);
	v->size = 5;
	while( v->count >= v->size )
		v->size *= 2;

	v->data = calloc( v->size, sizeof(reservation) );

	if( !(v->data) )
	{
		fputs( "Error allocating memory reading reservations from file.", stderr );
		snprintf( RES_ERROR_STR, BUFF, "Error reading reservations. Quitting the program." );
		// puts( "Error reading reservations from file. Quitting the program." );
		exit(1);
	}

	if( fread( v->data, sizeof(reservation), v->count, fp ) != v->count )
	{
		ERROR_RES( stderr, "Short read of data: fread");
		snprintf( RES_ERROR_STR, BUFF, "Error reading reservations. Quitting the program." );
		// puts( "Error reading reservations. Quitting the program" );
		exit(1);
		// fprintf( stderr, "%s:%d: Short read of data.\n", __FUNCTION__, __LINE__);
	}

	qsort( v->data, v->count, sizeof(reservation), res_sort_name_time );
	fclose( fp );
}

int resVect_sort_int( const void* left, const void* right )
{
	const int mleft = *(const int*)left;
	const int mright = *(const int*)right;
	return mleft - mright;
}

int resVect_sort_size_t( const void* left, const void* right )
{
	const size_t mleft = *(const size_t*)left;
	const size_t mright = *(const size_t*)right;
	if( mleft < mright )
		return -1;
	else if( mleft > mright )
		return 1;
	return 0;
}

int resVect_bsearch_room_cmp( const void* key, const void* element )
{
	const char* k = (const char*)key;
	const char* ele= *(const char**)element;
	// printf( "Inside bsearch_cmp: key is %s, with element %s\n", k, ele );
	return strcmp( k, ele );
}

int resVect_bsearch_time_cmp( const void* key, const void* element )
{
	const time_t* k = (const time_t*)key;
	const reservation* res = (const reservation*)element;
	// printf( "Key time is: %li\n", *k );
	time_t lower = res->starttime;
	time_t upper = res->endtime;
	// printf( "Reservation room: %s, start time: %li, end time: %li\n", res->roomname, lower, upper );
	if( *k < lower )
	{
		// puts( "Key is lower than starttime" );
		return -1;
	} else if( *k > upper ) {
		// puts( "Key is higher than endtime" );
		return 1;
	} else if( *k >= lower && *k <= upper ) {
		// puts( "MATCH" );
		return 0;
	}
}

int resVect_bsearch_day_cmp( const void* key, const void* element )
{
	const time_t* k = (const time_t*)key;
	const reservation* res = (const reservation*)element;
	time_t res_t_start = to_local( res->starttime );
	time_t res_t_end = to_local( res->endtime );
	struct tm restm_start;
	struct tm restm_end;
	localtime_r( &res_t_start, &restm_start );
	localtime_r( &res_t_end, &restm_end );
	struct tm ktm;
	localtime_r( k, &ktm );

	if( ktm.tm_wday < restm_start.tm_wday )
		return -1;
	else if( ktm.tm_wday > restm_end.tm_wday )
		return 1;
	return 0;
}

void resVect_check_consistency( resVect* v, char** rooms, int numrooms )
{
	char** checkname;

	// checkname = bsearch( "Billiard Room", rooms, numrooms, sizeof(char*), resVect_bsearch_room_cmp );
	// for( int i = 0; i < numrooms; i++)
	// {
	// 	printf( "Room %s at index %d\n", rooms[i], i );
	// }
	// printf( "Checkname is %s\n", checkname );
	for( int i = 0; i < v->count; i++ )
	{
		checkname = bsearch( v->data[i].roomname, rooms, numrooms, sizeof(char*), resVect_bsearch_room_cmp );
		
		// printf( "Checkname address at %s\n", checkname[0] );
		// size_t index = (checkname - rooms);
		// printf( "index at %li\n", index );

		if( checkname == NULL )
		{
			// ERROR_RES( stderr, "File inconsistency" );
			fprintf( stderr, "%s:%d: File incosistency. %s is missing from rooms.dat\n", __FUNCTION__, __LINE__, v->data[i].roomname );
			snprintf( RES_ERROR_STR, BUFF, "Inconsistent data in the reservation file. Quitting the program." );
			// puts( "Inconsistent data in the reservation file. Quitting the program" );
			exit(1);		
		}
	}
}

size_t* resVect_select_valid_rooms( resVect* v, time_t key, char** rooms, int numrooms )
{
	qsort( v->data, v->count, sizeof(reservation), res_sort_time_name );

	// for( int i = 0; i < v->count; i++ )
	// {
	// 	res_print_reservation( resVect_get( v, i ) );
	// }

	time_t timekey = to_utc( key );


	int index = 0;
	size_t reservedrooms[numrooms];
	reservation* reserved;
	reserved = bsearch( &timekey, v->data, v->count, sizeof(reservation), resVect_bsearch_time_cmp );

	size_t room_address;
	size_t res_address;
	size_t* available = NULL;
	if( reserved != NULL ) {
		// puts( "Found a reserved room." );
		// printf( "reservation is at address %li\n", reserved - v->data );
		char** foundroom = bsearch( reserved->roomname, rooms, numrooms, sizeof(char*), resVect_bsearch_room_cmp );
		room_address = foundroom - rooms;
		res_address = reserved - v->data;
		reservedrooms[index++] = room_address;

		// Go left
		size_t lefti = res_address - 1;
		// printf( "i is %li\n", lefti);
		// printf( "i: %li, starttime: %d, endtime: %d\n", lefti, v->data[lefti].starttime, v->data[lefti].endtime );
		while( lefti >= 0 && timekey >= v->data[lefti].starttime && timekey <= v->data[lefti].endtime )
		{
			// puts( "INSIDE WHILE LOOP");
			foundroom = bsearch( v->data[lefti].roomname, rooms, numrooms, sizeof(char*), resVect_bsearch_room_cmp );
			room_address = foundroom - rooms;
			reservedrooms[index++] = room_address;			
			lefti--;
		}

		// Go right
		size_t righti = res_address + 1;
		while( righti < v->count && timekey >= v->data[righti].starttime && timekey <= v->data[righti].endtime )
		{
			foundroom = bsearch( v->data[righti].roomname, rooms, numrooms, sizeof(char*), resVect_bsearch_room_cmp );
			room_address = foundroom - rooms;
			reservedrooms[index++] = room_address;
			righti++;
		}

		qsort( reservedrooms, index, sizeof(size_t), resVect_sort_int );
		available = calloc( (numrooms - index), sizeof(size_t) );
		if( !available )
		{
			fputs( "Error allocating memory to return available rooms.", stderr );
			snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations. Quitting the program." );
			// puts( "Error reading reservations from file. Quitting the program." );
			exit(1);
		}

		int avail_index = 0;
		for( size_t i = 0; i < numrooms; i++ )
		{
			if( bsearch( &i, reservedrooms, index, sizeof(size_t), resVect_sort_size_t ) == NULL )
				available[avail_index++] = i;

		}
		res_lookup_size = avail_index;
		// printf( "Number of reserved rooms %d\n", index );
		// puts( "PRINTING OUT FOUND INDEXES\n" );
		// for( int i = 0; i < index; i++ )
		// {

		// 	printf( "Room %s is reserved.\n", rooms[ reservedrooms[i] ]);
		// }
		// puts( "END INDEX PRINTING" );
		// puts( "\nPRINTING OUT AVAILABLE ROOMS\n" );
		// for( int i = 0; i < avail_index; i++ )
		// {

		// 	printf( "Room %s are available.\n", rooms[ available[i] ]);
		// }
		// puts( "END AVAILABLE PRINTING" );
		// res_print_reservation( reserved );
	}

	// qsort( v->data, v->count, sizeof(reservation), res_sort_name_time );
	return available;
}

size_t* resVect_select_res_day( resVect* v, time_t key )
{
	qsort( v->data, v->count, sizeof(reservation), res_sort_time_name );

	for( int i = 0; i < v->count; i++ )
	{
		res_print_reservation( resVect_get( v, i ) );
	}

	// time_t day_key = to_utc( key );

	int day_count = 0;
	int day_size = 5;
	size_t* res_on_day = NULL;
	reservation* res = NULL;

	res = bsearch( &key, v->data, v->count, sizeof(reservation),  resVect_bsearch_day_cmp );


	size_t res_index;
	if( res != NULL ) {
		puts( "Found a room on a day." );
		printf( "reservation is at index %li\n", res - v->data );
		res_print_reservation( res );
		res_on_day = calloc( day_size, sizeof(size_t) );
		if( !res_on_day )
		{
			fputs( "Error allocating memory to return reservations on a particular day.", stderr );
			snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations on a particular day. Quitting the program." );
			exit(1);
		}

		res_index = res - v->data;
		res_on_day[day_count++] = res_index;

		// Go left
		// printf( "i is %li\n", lefti);
		// printf( "i: %li, starttime: %d, endtime: %d\n", lefti, v->data[lefti].starttime, v->data[lefti].endtime );
		time_t res_t;
		struct tm res_tm;
		struct tm day_key_tm;
		localtime_r( &key, &day_key_tm );
		size_t lefti = res_index - 1;
		while( lefti >= 0 )
		{
			res_t = v->data[lefti].starttime;
			res_t = to_local( res_t );
			localtime_r( &res_t, &res_tm );
			printf( "LEFT: Key's day: %li, current reservation day: %li\n", day_key_tm.tm_wday, res_tm.tm_wday );
			if( res_tm.tm_wday != day_key_tm.tm_wday )
			{
				puts( "LEFT: DAYS NOT EQUAL" );
				break;
			}
			// puts( "INSIDE WHILE LOOP");
			if( day_count == day_size )
			{
				day_size *= 2;
				res_on_day = realloc( res_on_day, sizeof(size_t) * day_size );
				if( !res_on_day )
				{
					fputs( "Error reallocating memory to return reservations on a particular day.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations on a particular day. Quitting the program." );
					exit(1);	
				}
			}
			printf( "ADDING INDEX %li\n", lefti );
			res_on_day[day_count++] = lefti;
			// foundroom = bsearch( v->data[lefti].roomname, rooms, numrooms, sizeof(char*), resVect_bsearch_room_cmp );
			// room_index = foundroom - rooms;
			// reservedrooms[index++] = room_index;			
			lefti--;
		}

		// Go right
		size_t righti = res_index + 1;
		while( righti < v->count )
		{
			res_t = v->data[righti].starttime;
			res_t = to_local( res_t );
			localtime_r( &res_t, &res_tm );
			printf( "RIGHT: Key's day: %li, current reservation day: %li\n", day_key_tm.tm_wday, res_tm.tm_wday );
			if( res_tm.tm_wday != day_key_tm.tm_wday )
			{
				puts( "LEFT: DAYS NOT EQUAL" );
				break;
			}
			// puts( "INSIDE WHILE LOOP");
			if( day_count == day_size )
			{
				day_size *= 2;
				res_on_day = realloc( res_on_day, sizeof(size_t) * day_size );
				if( !res_on_day )
				{
					fputs( "Error reallocating memory to return reservations on a particular day.", stderr );
					snprintf( RES_ERROR_STR, BUFF, "Error retrieving available reservations on a particular day. Quitting the program." );
					exit(1);	
				}
			}
			printf( "ADDING INDEX %li\n", righti );
			res_on_day[day_count++] = righti;

			righti++;
		}

		qsort( res_on_day, day_count, sizeof(size_t), resVect_sort_int );

	}
	if( res_on_day )
		puts( "RES_ON_DAY has rooms" );
	else
		puts( "RES_ON_DAY is NULL" );
	res_lookup_size = day_count;
	return res_on_day;

}
