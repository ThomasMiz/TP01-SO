#ifndef _WORKER_HELPER_H_
#define _WORKER_HELPER_H_

#include <stdio.h>
#include "./../communication.h"

/**
 * Attempts to read another request from a given fd.
 * Blocks until a request is received, or the fd ends.
 * 
 * A new string is allocated for the received file's path and
 * this pointer is placed in filepathPtr. The user of this
 * function is responsible for freeing this string from memory
 * but only if the function returned 1.
 * 
 * Returns 1 if a request was read, 0 if end of file was reached.
 */
int readWorkerRequest(int fd, TWorkerRequest* request, char** filepathPtr);

/***
 * Interprets the output of the grep-ed minisat and writes
 * to the result struct cantidadVariables, cantidadClausulas,
 * timeNanoseconds and status.
 */
void interpretMinisatOutput(FILE* f, TWorkerResult* result);

#endif