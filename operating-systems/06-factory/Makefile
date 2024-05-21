CXX=clang++
CFLAGS=-Wall -g -O2
EXTRA_CFLAGS?=-fsanitize=address -fno-omit-frame-pointer
LIB=-lpthread

all: main.cpp
	$(CXX) -o factory main.cpp $(CFLAGS) $(EXTRA_CFLAGS) $(LIB)

.PHONY: clean zip

clean:
	rm -f $(TARGETS) *.o *.tgz

zip:
	tar czf osy06.tgz *.cpp Makefile
