APPS= crr 

CFLAGS+= -g -D_GNU_SOURCE -std=c99
LIBS= -L. -lattachable_debugger

include Makefile.so
include Makefile.hdeps

all:: ${APPS}

crr: crr.o reservation.o search_sort_utils.o crr_utils.o

clean:: 
	${RM} ${APPS} *.o *~

${APPS}: % : %.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

.PHONY: all clean

