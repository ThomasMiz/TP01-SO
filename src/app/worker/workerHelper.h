#ifndef _WORKER_HELPER_H_
#define _WORKER_HELPER_H_

#include <stdio.h>
#include "./../communication.h"

/**
 * Attempts to read another request from a given fd.
 * Blocks until a request is received, or the fd ends.
 * 
 * filepathBuf and filepathBufLen are the pointer to and size of
 * the buffer in which the filepath read will be placed. If this
 * buffer is not large enough, it will be realloc-ed.
 * 
 * Returns 1 if a request was read, 0 if end of file was reached.
 */
int readWorkerRequest(int fd, TWorkerRequest* request, char** filepathBuf, size_t* filepathBufLen);

/***
 * Interprets the output of the grep-ed minisat and writes
 * to the result struct cantidadVariables, cantidadClausulas,
 * timeNanoseconds and status.
 *
 * This function is a simple parser that expects something like this:
 * 100
 * 403
 * 0.00257
 * SATISFIABLE
 */
void interpretMinisatOutput(FILE* f, TWorkerResult* result);

#endif