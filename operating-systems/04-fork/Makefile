CC=gcc
CFLAGS=-g -Wall -Iinclude
EXTRA_CFLAGS=

BINARIES=forkpipe nsd
FORKPIPE_OBJ=forkpipe.o
NSD_OBJ=nsd_main.o nsd.o nd.o
NSD_HEADERS=nsd.h nd.h

all: forkpipe nsd

forkpipe: $(FORKPIPE_OBJ)
	$(CC) -o $@ $(FORKPIPE_OBJ)

nsd: $(NSD_OBJ) $(NSD_HEADERS)
	$(CC) -o $@ $(NSD_OBJ)

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $(EXTRA_CFLAGS) $<

.PHONY: clean zip

clean:
	rm -f $(BINARIES) *.o *.tgz

zip:
	tar czf osy04.tgz *.[ch] Makefile