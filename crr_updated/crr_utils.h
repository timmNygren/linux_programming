#ifndef CRR_UTILS_H
#define CRR_UTILS_H

#define NUM_RESERVATIONS 5
#define TRUE 1
#define FALSE 0
#define BUFFLEN 1024

extern const char* MAIN_MENU[];
extern const char* VALID_DATE_FORMATS[];
// extern const char* RESERVATION_MENU[];

void main_menu( WINDOW *menu_win, int highlight );
int print_format_list( WINDOW* displayWin );
void print_rooms( char** roomnames, int numRooms, int printNums );
void crr_print_menu( char** menu, size_t* lookups, int lookups_size, int printNums );
reservation new_reservation( char* roomname );
reservation* crr_update_reservation( char* roomname, resVect* v, int res_pos );
void crr_print_reservations( resVect* v, size_t* lookups, int lookups_size );

#endif
