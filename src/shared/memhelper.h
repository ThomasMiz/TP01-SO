#ifndef _MEMHELPER_H_
#define _MEMHELPER_H_

#include <stddef.h>
#include <semaphore.h>

/**
 * Provides some basic wrappers around memory management functions
 * to help with allocating memory.
 */

/**
 * Attempts to malloc(size). If the operation succeeds, the resulting
 * pointer is placed in *ptr and 1 is returned. Otherwise, ptr is left
 * unchanged and 0 is returned.
 */
int tryMalloc(void** ptr, size_t size);

/**
 * Attempts to realloc(*ptr, size). If the operation succeeds, the
 * resulting pointer is placed in *ptr and 1 is returned. Otherwise,
 * ptr is left unchanged and 0 is returned.
 */
int tryRealloc(void** ptr, size_t size);

/**
 * Attempts to calloc(nmemb, size). If the operation succeeds, the
 * resulting pointer is placed in *ptr and 1 is returned. Otherwise,
 * ptr is left unchanged and 0 is returned.
 */
int tryCalloc(void** ptr, size_t nmemb, size_t size);

/**
 * If *size < requiredSize, attempts to realloc(*ptr, requiredSize) and
 * places the resulting pointer in *ptr. If this fails, 0 is returned.
 * Otherwise, 1 is returned.
 */
int tryReallocIfNecessary(void** ptr, size_t* size, size_t requiredSize);

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

/**
 * A wrapper around sem_wait() which ensures the operation does not
 * return preemtively in case of a signal interrupt.
 *
 * Returns 1 if the operation succeeded, 0 if errors were encountered.
 */
int sem_wait_nointr(sem_t* sem);

#endif