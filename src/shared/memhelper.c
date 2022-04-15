// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "memhelper.h"
#include "constants.h"

#define READ_MAX_FAILS 3
#define WRITE_MAX_FAILS 3

int tryMalloc(void** ptr, size_t size)
{
    void* newPtr = malloc(size);
    if (newPtr == NULL)
        return 0;

    *ptr = newPtr;
    return 1;
}

int tryRealloc(void** ptr, size_t newSize)
{
    void* newPtr = realloc(*ptr, newSize);
    if (newPtr == NULL)
        return 0;

    *ptr = newPtr;
    return 1;
}

int tryCalloc(void** ptr, size_t nmemb, size_t size)
{
    void* newPtr = calloc(nmemb, size);
    if (newPtr == NULL)
        return 0;

    *ptr = newPtr;
    return 1;
}

int tryReallocIfNecessary(void** ptr, size_t* size, size_t requiredSize) {
	return (*size >= requiredSize && *ptr != NULL) ? 1 : tryRealloc(ptr, requiredSize);
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