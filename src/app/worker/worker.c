#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "worker.h"
#include "./../communication.h"
#include "./../constants.h"
#include "./../../shared/memhelper.h"

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
int readWorkerRequest(int fd, TWorkerRequest* request, char** filepathPtr) {
	unsigned int bytesRead = 0;
	
	do {
		int r = read(fd, (void*)request + bytesRead, sizeof(TWorkerRequest) - bytesRead);
		
		if (r == 0) {
			if (bytesRead)
				fprintf(stderr, "Worker error: request pipe closed mid-request during TWorkerRequest read.\n");
			return 0; // Nothing else to read from this fd.
		}
		
		if (r < 0)
			continue; // Read failed, let's try again.
		
		bytesRead += r;
		
	} while(bytesRead < sizeof(TWorkerRequest));
	
	// The TWorkerRequest struct has been read. Now comes the filepath!
	char* s = mallocOrExit(request->filepathLength + 1);
	
	bytesRead = 0;
	do {
		int r = read(fd, s + bytesRead, request->filepathLength - bytesRead);
		
		if (r == 0) {
			fprintf(stderr, "Worker error: request pipe closed mid-request during filepath read.\n");
			free(s);
			return 0;
		}
		
		if (r < 0)
			continue; // Read failed, let's try again.
		
		bytesRead += r;
	} while (bytesRead < request->filepathLength);
	
	s[request->filepathLength] = '\0';
	*filepathPtr = s;
	return 1;
}

int main(int argc, const char* argv[]) {
	
	if (argc <= 1) {
		fprintf(stderr, "Worker Error: not enough parameters. Workers must receive their id as their only parameter.\n");
		exit(EXIT_CODE_NOT_ENOUGH_PARAMS);
	}
	
	unsigned int workerId = atoi(argv[1]);
	
	TWorkerRequest request;
	char* filepath;
	while (readWorkerRequest(STDIN_FILENO, &request, &filepath)) {
		free(filepath);
		
		TWorkerResult result;
		result.taskId = request.taskId;
		result.status = Sat;
		result.cantidadClausulas = workerId;
		result.cantidadVariables = request.filepathLength;
		result.timeNanoseconds = 0;
		write(STDOUT_FILENO, &result, sizeof(TWorkerResult)); // TODO: wrap to ensure all bytes written
	}
	
	exit(0);
}