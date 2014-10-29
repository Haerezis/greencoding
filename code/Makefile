CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused-result
LDFLAGS=-fopenmp -lpthread -lrt

.PHONY: all clean distclean

all: release

debug: CFLAGS += -g -pg
debug: LDFLAGS += -g -pg
debug: code

release: CFLAGS += -O3
release: LDFLAGS += -O3
release: code

code: lightupsolver.o lightup.o utils.o whbufs.o heuristics.o solve.o dvfs_core.o dvfs_unit.o dvfs_context.o dvfs_error.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o code

archive:
#	cd .. && tar czf code_upload.tar.gz code/*
	tar czf code_upload.tar.gz *.c *.h Makefile
