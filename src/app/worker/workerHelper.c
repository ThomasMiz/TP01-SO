#include <stdlib.h>
#include "workerHelper.h"
#include "./../../shared/memhelper.h"

int readWorkerRequest(int fd, TWorkerRequest* request, char** filepathBuf, size_t* filepathBufLen) {
	
	if (!readFull(fd, request, sizeof(TWorkerRequest)))
		return 0; // End of file reached.
	
	// The TWorkerRequest struct has been read. Now comes the filepath!
	
	// We ensure the filepath buffer is large enough to hold the string and a '\0'.
	if (!tryReallocIfNecessary((void**)filepathBuf, filepathBufLen, request->filepathLength + 1)) {
		fprintf(stderr, "[Worker] Error: Failed to alloc for filepath length %u.\n", request->filepathLength);
		return 0;
	}
	
	if (!readFull(fd, *filepathBuf, request->filepathLength)) {
		fprintf(stderr, "[Worker] Error: Failed to read filepath of length %u. Did pipe close?\n", request->filepathLength);
		return 0;
	}
	
	// We null-terminate the string and return 1 for success.
	(*filepathBuf)[request->filepathLength] = '\0';
	return 1;
}

/**
 * Reads from f. Discards chars until a digit is found, then reads
 * that integer number (until the next non-digit char is found).
 *
 * Returns 1 if successfull, 0 if end of file was reached before
 * a digit was found.
 */
static int readAnotherInt(FILE* f, unsigned int* result) {
	int c;
	do { c = fgetc(f); } while ((c < '0' || c > '9') && c != EOF);
	if (c == EOF) return 0;
	
	unsigned int r = 0;
	do {
		r = r * 10 + c - '0';
		c = fgetc(f);
	} while (c >= '0' && c <= '9');
	
	*result = r;
	return 1;
}

static int readTimeNanoseconds(FILE* f, unsigned long* nanos) {
	// f is expected to contain something like this:
	// CPU time              : 0.002541 s
	
	int c;
	do { c = fgetc(f); } while ((c < '0' || c > '9') && c != EOF);
	if (c == EOF) return 0;
	
	unsigned long t = c - '0';
	int multsByTen = 9;
	
	// Read the integer part:
	while ((c = fgetc(f)) >= '0' && c <= '9') { t = t * 10 + c - '0'; }
	// The '.' in the number is skipped in between these two loops.
	// Read the decimal part:
	while ((c = fgetc(f)) >= '0' && c <= '9') { t = t * 10 + c - '0'; multsByTen--; }
	
	// Do remaining multiply-by-tens to leave the number in nanoseconds.
	for (; multsByTen > 0; multsByTen--) { t *= 10; }
	
	*nanos = t;
	return 1;
}

static enum SatResult readIsSat(FILE* f) {
	// Advance f until the char read is 'S', 'U', or end was reached.
	int c;
	do { c = fgetc(f); } while (c != 'U' && c != 'S' && c != EOF);
	
	return c == 'S' ? Sat : (c == 'U' ? Unsat : Error);
}

void interpretMinisatOutput(FILE* f, TWorkerResult* result) {
	/*
	This function is a simple parser for outputs of the grep-ed minisat.
	It expects the stream f to bring an output that looks like this:

Number of variables:           100
Number of clauses:             403
CPU time              : 0.002541 s
SATISFIABLE

	It will just look for the first number and set it to cantidadVariables,
	repeat for cantidadVariables, then repeat for timeNanoseconds but
	parsing that numeric value into nanoseconds. Finally, it will advance
	until it finds either the char 'S' or 'U' to decide if the result is
	SATISFIABLE or UNSATISFIABLE.
	If any of that fails (eg. end of file is reached unexpectedly), sets
	the result as "Error".
	*/
	
	result->cantidadClausulas = 0;
	result->cantidadVariables = 0;
	result->timeNanoseconds = 0;
	
	if (readAnotherInt(f, &result->cantidadVariables) && readAnotherInt(f, &result->cantidadClausulas)
		&& readTimeNanoseconds(f, &result->timeNanoseconds)) {
		result->status = readIsSat(f);
	} else {
		result->status = Error;
	}
}