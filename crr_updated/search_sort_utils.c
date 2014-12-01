#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "reservation.h"
#include "search_sort_utils.h"

int sort_name_time( const void* left, const void* right )
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

int sort_time_name( const void* left, const void* right )
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

int sort_int( const void* left, const void* right )
{
	const int mleft = *(const int*)left;
	const int mright = *(const int*)right;
	return mleft - mright;
}

int sort_size_t( const void* left, const void* right )
{
	const size_t mleft = *(const size_t*)left;
	const size_t mright = *(const size_t*)right;
	if( mleft < mright )
		return -1;
	else if( mleft > mright )
		return 1;
	return 0;
}

int bsearch_room_cmp( const void* key, const void* element )
{
	const char* k = (const char*)key;
	const char* ele= *(const char**)element;

	return strcmp( k, ele );
}

int bsearch_time_cmp( const void* key, const void* element )
{
	const time_t* k = (const time_t*)key;
	const reservation* res = (const reservation*)element;

	time_t lower = res->starttime;
	time_t upper = res->endtime;

	if( *k < lower )
	{
		return -1;
	} else if( *k > upper ) {
		return 1;
	} else if( *k >= lower && *k <= upper ) {
		return 0;
	}
}

int bsearch_day_cmp( const void* key, const void* element )
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

int bsearch_conflict( const void* key, const void* element )
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
