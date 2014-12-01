#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

int sort_name_time( const void* left, const void* right );
int sort_time_name( const void* left, const void* right );
int sort_int( const void* left, const void* right );
int sort_size_t( const void* left, const void* right );
int bsearch_room_cmp( const void* key, const void* element );
int bsearch_time_cmp( const void* key, const void* element );
int bsearch_day_cmp( const void* key, const void* element );
int bsearch_conflict( const void* key, const void* element );

#endif
