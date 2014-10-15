CC=gcc
CFLAGS=-O2 -Wall -Wextra
LDFLAGS=-O2

.PHONY: all clean distclean

all: lightupchecker 

lightupchecker: lightupchecker.o lightup.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f lightupchecker
