CC=gcc
CFLAGS=-O3 -Wall -Wextra -fopenmp
LDFLAGS=-O3 -fopenmp

.PHONY: all clean distclean

all: code

code: lightupsolver.o lightup.o utils.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f code

archive:
	tar czf code_upload.tar.gz *.c *.h Makefile
