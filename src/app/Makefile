include ./../Makefile.inc

SOURCES=$(wildcard *.c)
WORKER_SOURCES=$(wildcard ./worker/*.c)
SOURCES_SHARED=$(wildcard ./../shared/*.c)
OUTPUT_FILE=./../solve.out
WORKER_OUTPUT_FILE=./../worker.out

all:
	$(GCC) $(GCCFLAGS) $(SOURCES) $(SOURCES_SHARED) -o $(OUTPUT_FILE)
	$(GCC) $(GCCFLAGS) $(WORKER_SOURCES) $(SOURCES_SHARED) -o $(WORKER_OUTPUT_FILE)

clean:
	rm -f $(OUTPUT_FILE)
	rm -f $(WORKER_OUTPUT_FILE)

.PHONY: all clean