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

#endif