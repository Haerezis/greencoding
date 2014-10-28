CC=gcc
CFLAGS=-Wall -Wextra -fPIC -g
LDFLAGS=-fopenmp -lpthread -lrt -g

LDFLAGS=

# Setup variables
PREFIX?=/usr/local
INCLUDE_DIR?=$(PREFIX)/include
LIB_DIR?=$(PREFIX)/lib


.PHONY: all clean distclean

all: code libdvfs.so

release: CFLAGS += -O3
release: code

code: lightupsolver.o lightup.o utils.o whbufs.o heuristics.o solve.o libdvfs.so
	$(CC) $^ -o $@ $(LDFLAGS)  -Wl,-rpath=libdvfs -L -llibdvfs

libdvfs.so: dvfs_core.o dvfs_unit.o dvfs_context.o dvfs_error.o
	$(CC) -shared $(CFLAGS) $^ $(LDFLAGS) -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

distclean: clean
	rm -f code libdvfs.so

archive:
	tar czf code_upload.tar.gz *.c *.h Makefile
