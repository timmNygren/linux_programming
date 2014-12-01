CFLAGS+= -g -D_GNU_SOURCE -std=c99
LIBS=-lcurses
all: crr
#all: swc

#${APPS}: % : %.c
# rule above turns into 
#	stdfiles: stdfiles.c
#	stringstring: stringstring.c
#	wordat: wordat.c

crr: crr.o reservation.o crr_utils.o

crr.o reservation.o crr_utils.o: reservation.h crr_utils.h

charcell: ${LIBS}
#charcell: charcell.o
#	${CC} ${LDFLAGS} -o charcell charcell.o -lcurses

charcell-resize: ${LIBS}
#charcell-resize: charcell-resize.o
#	${CC} ${LDFLAGS} -o charcell-resize charcell-resize.o -lcurses

signal_example: signal_example.o signal_example_utils.o


.PHONY: all clean
clean:
#	${RM} swc *.o
	${RM} crr *.o
