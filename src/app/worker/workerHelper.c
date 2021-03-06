// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

void interpretMinisatOutput(FILE* f, TWorkerResult* result) {
	result->cantidadClausulas = 0;
	result->cantidadVariables = 0;
	result->timeSeconds = 0;
	
	char  c;
	int matches = fscanf(f, "%10u\n%10u\n%16lf\n%c", &result->cantidadClausulas, &result->cantidadVariables, &result->timeSeconds, &c);
	
	if (matches < 4) {
		result->status = Error;
	} else {
		result->status = (c == 'S' ? Sat : (c == 'U' ? Unsat : Error));
	}
}