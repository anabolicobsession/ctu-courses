CC=gcc
CFLAGS=-Wall -g -O2
EXTRA_CFLAGS?=-fsanitize=address -fno-omit-frame-pointer

TARGETS=prod-cons
OBJ=prod-cons.o linked_list.o

all: $(TARGETS)

prod-cons: $(OBJ)
	$(CC) -o $@ $(OBJ) $(EXTRA_CFLAGS) -lpthread

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $(EXTRA_CFLAGS) $<

.PHONY: clean zip

clean:
	rm -f $(TARGETS) *.o *.tgz

zip:
	tar czf osy05.tgz *.[ch] Makefile
