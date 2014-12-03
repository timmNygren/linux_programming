#ifndef RESERVATION_H
#define RESERVATION_H

#define BUFF 1024
#define DESC_SIZE 129
#define ROOM_NAME_LEN 49
#define ARRAY_OUT_OF_BOUNDS -1

extern char RES_ERROR_STR[BUFF];
extern int res_lookup_size;

typedef struct Reservation {
	char roomname[ROOM_NAME_LEN];
	time_t starttime;
	time_t endtime;
	char description[DESC_SIZE];
} reservation;

reservation create_reservation( const char* roomname, const time_t start, const time_t end, const char* desc );
reservation* update_reservation( reservation* oldreservation, const char* newroomname, const time_t newstart, const time_t newend, const char* newdesc );
void res_print_reservation( reservation* res );
time_t to_local( time_t t );
time_t to_utc( time_t t );

typedef struct Reservation_Vector {
	reservation* data;
	int size;
	int count;
} resVect;

void resVect_init( resVect* v );
int resVect_count( resVect* v );
reservation* resVect_add( resVect* v, reservation res );
void resVect_set( resVect* v, int index, reservation res );
reservation* resVect_get( resVect* v, int index );
void resVect_delete( resVect* v, int index );
void resVect_free( resVect* v );
void resVect_write_file( resVect* v, char* filename );
void resVect_read_file( resVect* v, char* filename );
void resVect_check_consistency( resVect* v, char** rooms, int numrooms );
size_t* resVect_select_room_at_time( resVect* v, time_t key, char** rooms, int numrooms );
size_t* resVect_select_res_day( resVect* v, time_t key );
size_t* resVect_select_res_room( resVect* v, char* key );
size_t* resVect_select_res_desc( resVect* v, char* key );

#endif
