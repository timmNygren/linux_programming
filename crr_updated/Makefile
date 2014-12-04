APPS=  crr test_get_str
APPS+= charcell-resize
APPS+= charcell-signal-kip
APPS+= charcell-atexit

CFLAGS+= -g -D_GNU_SOURCE -std=c99
LIBS= -L. -lattachable_debugger
#LDFLAGS+= -Wl, --no-as-needed

include Makefile.so
include Makefile.hdeps

all:: ${APPS}


# the charcell application needs the ncurses library, the other apps don't
charcell-resize: LIBS+=-lncurses
charcell-signal-kip: LIBS+=-lncurses
charcell-atexit: LIBS+=-lncurses
charcell-signal-kip: charcell-utils.o
test_get_str: LIBS+=-lncurses
test_get_str: charcell-utils.o


charcell-atexit: charcell-atexit.o charcell-atexit-keyhistory.o charcell-utils.o
crr: crr.o reservation.o search_sort_utils.o crr_utils.o	

clean:: 
	${RM} ${APPS} *.o *~

# a static-pattern rule for all APPS
${APPS}: % : %.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

# LDFLAGS - rule to build application to do linking
# $@ - target you are building (ie. %)
# $^ - all dependencies, unique (ie. swc.o swcutils.o)

#SOFLAGS+= -shared
#LD= cc
# %.so:
#	${LD} ${SOFLAGS} -o ${@F} $^ ${SOLIBS}
# LD - linker
# @F - just the target name itself, no path
# 

.PHONY: all clean

