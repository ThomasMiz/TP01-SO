#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "memhelper.h"
#include "constants.h"

#define READ_MAX_FAILS 3
#define WRITE_MAX_FAILS 3

void* mallocOrExit(size_t size) {
	void* ptr = malloc(size);
	
	if (ptr == NULL && size != 0) {
		fprintf(stderr, "Not enough memory: failed to allocate %lu bytes.\n", size);
		exit(EXIT_CODE_NOT_ENOUGH_MEMORY);
	}
	
	return ptr;
}

void* reallocOrExit(void* ptr, size_t size) {
	ptr = realloc(ptr, size);
	
	if (ptr == NULL && size != 0) {
		fprintf(stderr, "Not enough memory: failed to allocate %lu bytes.\n", size);
		exit(EXIT_CODE_NOT_ENOUGH_MEMORY);
	}
	
	return ptr;
}

void* callocOrExit(size_t nmemb, size_t size) {
	void* ptr = calloc(nmemb, size);
	
	if (ptr == NULL && size != 0) {
		fprintf(stderr, "Not enough memory: failed to allocate %lu bytes.\n", size);
		exit(EXIT_CODE_NOT_ENOUGH_MEMORY);
	}
	
	return ptr;
}

int writeFull(int fd, const void* buf, size_t count) {
	unsigned int fails = 0;
	
	size_t bytesWritten = 0;
	do {
		ssize_t r = write(fd, buf + bytesWritten, count - bytesWritten);
		
		// We handle any errors that could occur from the write.
		if (r < 0) {
			// If the write failed because it was interrupted by a signal,
			// we try again but don't count it as a fail.
			if (errno != EINTR && ++fails >= WRITE_MAX_FAILS)
				return 0; // Too many fails, return an error.
		} else {
			bytesWritten += r;
		}
		
	} while(bytesWritten < count);
	
	return 1;
}

int readFull(int fd, void* buf, size_t count) {
	unsigned int fails = 0;
	
	size_t bytesRead = 0;
	do {
		ssize_t r = read(fd, buf + bytesRead, count - bytesRead);
		
		// We handle any errors that could occur from the read.
		if (r < 0) {
			// If the read failed because it was interrupted by a signal,
			// we try again but don't count it as a fail.
			if (errno != EINTR && ++fails >= READ_MAX_FAILS)
				return 0; // Too many fails, return an error.
		} else if (r == 0) {
			// If the read failed because end of file was reached,
			// we return an error.
			return 0;
		} else {
			bytesRead += r;
		}
		
	} while(bytesRead < count);
	
	return 1;
}