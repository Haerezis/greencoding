CC=gcc
CFLAGS=-O2 -g -Wall -Wextra
LDFLAGS=-O2 -g

.PHONY: all clean distclean

all: lightupgen

lightupgen: lightupgen.o lightup.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f lightupgen
