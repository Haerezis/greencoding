CC=gcc
CFLAGS=-O3 -Wall -Wextra
LDFLAGS=-O3 

.PHONY: all clean distclean

all: code

code: lightupsolver.o lightup.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f code
