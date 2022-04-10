#ifndef _MEMHELPER_H_
#define _MEMHELPER_H_

#include <stddef.h>

#define EXIT_CODE_NOT_ENOUGH_MEMORY 4

/**
 * Provides some basic wrappers around memory management functions
 * to help with allocating memory.
 */

/**
 * Same as malloc(size), but exits the process if the operation fails.
 */
void* mallocOrExit(size_t size);

/**
 * Same as realloc(ptr, size), but exits the process if the operation fails.
 */
void* reallocOrExit(void* ptr, size_t size);

/**
 * Same as calloc(nmemb, size), but exits the process if the operation fails.
 */
void* callocOrExit(size_t nmemb, size_t size);

/**
 * A wrapper around write() which ensures the entire buffer is written to
 * the fd, since normal write() may not write as many bytes as requested.
 *
 * Returns 1 if the operation succeeded, 0 if errors were encountered.
 */
int writeFull(int fd, const void* buf, size_t count);

/**
 * A wrapper around read() which ensures the entire buffer is read from
 * the fd, since normal read() may not read as many bytes as requested.
 *
 * Returns 1 if the operation succeeded, 0 if errors were encountered.
 */
int readFull(int fd, void* buf, size_t count);

#endif