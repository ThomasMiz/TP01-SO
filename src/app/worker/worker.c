#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
	
	if (!readFull(fd, request, sizeof(TWorkerRequest))) {
		return 0; // End of file reached.
	}
	
	// The TWorkerRequest struct has been read. Now comes the filepath!
	char* s = mallocOrExit(request->filepathLength + 1);
	
	if (!readFull(fd, s, request->filepathLength)) {
		fprintf(stderr, "Worker error: failed to read filepath of length %u.\n", request->filepathLength);
		free(s); // Free the string, the user is only responsible for it when we return 1.
		return 0;
	}
	
	// We null-terminate the string and place it in the output param.
	s[request->filepathLength] = '\0';
	*filepathPtr = s;
	return 1;
}

int main(int argc, const char* argv[]) {
	
	TWorkerRequest request;
	char* filepath;
	while (readWorkerRequest(STDIN_FILENO, &request, &filepath)) {
		free(filepath);
		
		TWorkerResult result;
		result.taskId = request.taskId;
		result.status = Sat;
		result.cantidadClausulas = request.filepathLength;
		result.cantidadVariables = 420;
		result.timeNanoseconds = 0;
		write(STDOUT_FILENO, &result, sizeof(TWorkerResult)); // TODO: wrap to ensure all bytes written
	}
	
	exit(0);
}