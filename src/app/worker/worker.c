// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "workerHelper.h"
#include "./../communication.h"
#include "./../../shared/constants.h"
#include "./../../shared/memhelper.h"

#define CMD_TEMPLATE_1 "minisat \""
#define CMD_TEMPLATE_2 "\" | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\" | grep -o -e \"[0-9\\.]*\" -e \".*SATISFIABLE\""

/**
 * Attempts to run the minisat and output a result.
 * Returns 0 if a fatal error ocurred, 1 otherwise.
 */
static int runMinisat(const TWorkerRequest* request, TWorkerResult* result, char** filepathBuf, size_t* filepathBufLen) {
	size_t template1Length = strlen(CMD_TEMPLATE_1);
	size_t template2Length = strlen(CMD_TEMPLATE_2);
	
	// We will use filepathBuf as a buffer to store the whole command.
	// We ensure it's large enough to hold the whole command plus a '\0'.
	if (!tryReallocIfNecessary((void**)filepathBuf, filepathBufLen, request->filepathLength + template1Length + template2Length + 1)) {
		fprintf(stderr, "[Worker] Error: Failed to alloc for cmd length %u.\n", request->filepathLength);
		return 0;
	}
	
	// We use this buffer to form the minisat execution command.
	// This is composed of the concatenation of the strings CMD_TEMPLATE_1,
	// the file path, and CMD_TEMPLATE_2.
	char* buf = *filepathBuf;
	
	memmove(buf + template1Length, buf, request->filepathLength);
	memcpy(buf, CMD_TEMPLATE_1, template1Length);
	strcpy(buf + request->filepathLength + template1Length, CMD_TEMPLATE_2);

	FILE* f = popen(buf, "r");
	
	if (f == NULL) {
		fprintf(stderr, "[Worker] Error: Failed to run popen() with: %s\n", buf);
		perror("Reason");
		return 0;
	}
	
	interpretMinisatOutput(f, result);
	
	pclose(f);
	return 1;
}

int main(int argc, const char* argv[]) {
	TWorkerRequest request;
	TWorkerResult result;
	
	int exitCode = 0;
	
	// Buffer used for the file paths and commands.
	char* buf = NULL;
	size_t bufLen = 0;
	
	while (readWorkerRequest(STDIN_FILENO, &request, &buf, &bufLen)) {
		// We attempt to run minisat and get an output result. If a fatal
		// error occurs, we break to free remaining memory and exit.
		if (!runMinisat(&request, &result, &buf, &bufLen)) {
			exitCode = EXIT_FAILURE;
			break;
		}
		
		result.taskId = request.taskId;
		writeFull(STDOUT_FILENO, &result, sizeof(TWorkerResult));
	}
	
	free(buf);
	return exitCode;
}