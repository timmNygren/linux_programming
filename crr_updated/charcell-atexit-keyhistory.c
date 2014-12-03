/***
 * keystroke history written to file on atexit() for
 * charcell-atexit.c
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h> // atexit

#define BUFLEN 1024
#define HISTORY 32
char key_history[HISTORY][BUFLEN];
int history = 0;

void store_key_history( void )
{
	if( history == 0 &&  !key_history[0][0] ) {
		// nothing written
		return;
	}

	FILE* fp = fopen( "KeyHistory.dat", "a" );
	// scan to first entry
	do {
		++history;
		history %= HISTORY;
	} while( !key_history[history][0] );

	int start = history;
	do {
		fputs( key_history[history], fp );
		fputs( "\n", fp );
		++history;
		history %= HISTORY;
	} while( history != start && key_history[history][0] );

    	fputs( "\n", fp );
	fclose(fp);
}

void setup_key_history( void )
{
	atexit( store_key_history );
	// terminate all entries
	for( int i=0; i<HISTORY; i++ ) {
		key_history[i][0] = '\0';
	}
}

void add_key_history( const char* const buf )
{
	strncpy( key_history[history], buf, BUFLEN );
	key_history[history][BUFLEN-1] = '\0';  // be sure
    	// next!
	++history;
	history %= HISTORY;
}


