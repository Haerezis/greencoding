CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused-function
LDFLAGS=-fopenmp -g

.PHONY: all clean distclean

all: debug

release: CFLAGS += -O3
release: code


debug: CFLAGS += -DDEBUG -g
debug: code

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
