include ./../Makefile.inc

SOURCES=$(wildcard *.c)
SOURCES_SHARED=$(wildcard ./../shared/*.c)
OUTPUT_FILE=./../../vista

all:
	$(GCC) $(GCCFLAGS) $(SOURCES) $(SOURCES_SHARED) -o $(OUTPUT_FILE)

clean:
	rm -f $(OUTPUT_FILE)

.PHONY: all clean