#include <stdlib.h>
#include "workerHelper.h"
#include "./../../shared/memhelper.h"

int readWorkerRequest(int fd, TWorkerRequest* request, char** filepathPtr) {
	
	if (!readFull(fd, request, sizeof(TWorkerRequest))) {
		return 0; // End of file reached.
	}
	
	// The TWorkerRequest struct has been read. Now comes the filepath!
	char* s = mallocOrExit(request->filepathLength + 1);
	
	if (!readFull(fd, s, request->filepathLength)) {
		fprintf(stderr, "[Worker] Error: Failed to read filepath of length %u.\n", request->filepathLength);
		free(s); // Free the string, the user is only responsible for it when we return 1.
		return 0;
	}
	
	// We null-terminate the string and place it in the output param.
	s[request->filepathLength] = '\0';
	*filepathPtr = s;
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
	int c;
	do { c = fgetc(f); } while ((c < '0' || c > '9') && c != EOF);
	if (c == EOF) return 0;
	
	unsigned long t = c - '0';
	int multsByTen = 9;
	
	while ((c = fgetc(f)) >= '0' && c <= '9') { t = t * 10 + c - '0'; }
	// The '.' is found in between these two loops
	while ((c = fgetc(f)) >= '0' && c <= '9') { t = t * 10 + c - '0'; multsByTen--; }
	
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