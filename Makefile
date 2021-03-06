# Makefile for Process Library

XDRGEN?=poly-xdrgen
PLAT=$(shell uname -s)
include Make.rules.$(PLAT)
include Make.rules.arm

# Input/Output Variables
SOURCES=priorityQueue.c events.c proclib.c ipc.c debug.c cmd.c config.c hashtable.c util.c md5.c critical.c eventTimer.c telm_dict.c zmqlite.c json.c cmd-pkt.c xdr.c plugin.c
LIBRARY_NAME=proc
MAJOR_VERS=3
MINOR_VERS=0.1

# Install Variables
INCLUDE=proclib.h events.h ipc.h config.h debug.h cmd.h polysat.h hashtable.h util.h md5.h priorityQueue.h eventTimer.h telm_dict.h zmqlite.h critical.h xdr.h cmd-pkt.h plugin.h

# Build Variables
override CFLAGS+=$(SYMBOLS) -Wall -Werror -std=gnu99 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 $(SO_CFLAGS)
override LDFLAGS+= -ldl
SRC_PATH=.

# Private Variables
OBJECTS=$(SOURCES:.c=.o)
LIBRARY=lib$(LIBRARY_NAME)
SO_NAME=$(LIBRARY).$(SO_EXT).$(MAJOR_VERS)
LIB_NAME=$(LIBRARY).$(SO_EXT).$(MAJOR_VERS).$(MINOR_VERS)
LFLAGS=-shared -fPIC -Wl,-soname,$(SO_NAME)

all: $(OBJECTS) $(LIBRARY)

$(LIBRARY): $(LIB_NAME)
	 ln -sf $(LIB_NAME) $(LIBRARY).$(SO_EXT)
	 ln -sf $(LIB_NAME) $(LIBRARY).$(SO_EXT).$(MAJOR_VERS)

$(LIB_NAME): $(OBJECTS)
	 $(CC) $(SO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -lpthread -o $(LIB_NAME)

cleanup_test: cleanup_test.c util.c priorityQueue.c debug.c
	 $(CC) $(CFLAGS) cleanup_test.c util.c priorityQueue.c debug.c $(LDFLAGS) -o $@

cmd.c: cmd-pkt.h
ipc.c: cmd-pkt.h

cmd-pkt.h: cmd-pkt.xp
	$(XDRGEN) --target libproc --output cmd-pkt cmd-pkt.xp

cmd-pkt.c: cmd-pkt.xp
	$(XDRGEN) --target libproc --output cmd-pkt cmd-pkt.xp


install: all
	cp $(LIB_NAME) $(LIB_PATH)
	$(INST_STRIP) $(LIB_PATH)/$(LIB_NAME)
	ln -sf $(LIB_NAME) $(LIB_PATH)/$(LIBRARY).$(SO_EXT)
	ln -sf $(LIB_NAME) $(LIB_PATH)/$(LIBRARY).$(SO_EXT).$(MAJOR_VERS)
	install -d $(INC_PATH)/polysat
	cp $(INCLUDE) $(INC_PATH)/polysat
#	ldconfig -linux-ld -n $(LIB_PATH)

uninstall:
	rm $(LIB_PATH)/$(LIBRARY)*
	rm -rf $(INC_PATH)/polysat

test:
	make -C ./tests/unit/
	./tests/unit/tests 2> /dev/null

.c.o:
	 $(CC) $(CFLAGS) -c $(SRC_PATH)/$< -o $@

clean:
	rm -rf *.o $(LIBRARY).$(SO_EXT)* $(LIBRARY) cleanup_test cmd-pkt.c cmd-pkt.h
