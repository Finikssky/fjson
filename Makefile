CC=gcc
CFLAGS=-g
IFLAGS=-I.
LDFLAGS=-L.

OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

%.o: %.c
	$(CC) $(CFLAGS) $(IFLAGS) -fPIC -c -o $@ $<

all: $(OBJS)
	$(CC) $(CFLAGS) $(IFLAGS) -shared *.o -o libfjson.so

test: test.o
	$(CC) $(CFLAGS) $(IFLAGS) $(LDFLAGS) test.o -o test -lfjson -lavl

clean:
	rm -f *.o
	rm -f libfjson.so