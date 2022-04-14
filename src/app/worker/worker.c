#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "workerHelper.h"
#include "./../communication.h"
#include "./../../shared/constants.h"
#include "./../../shared/memhelper.h"

#define CMD_TEMPLATE_1 "minisat \""
#define CMD_TEMPLATE_2 "\" | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\""
#define CMD_TEMPLATE_1_LEN 9
#define CMD_TEMPLATE_2_LEN 70

/**
 * Attempts to run the minisat and output a result.
 * Returns 0 if a fatal error ocurred, 1 otherwise.
 */
static int runMinisat(const TWorkerRequest* request, TWorkerResult* result, char** filepathBuf, size_t* filepathBufLen) {
	// We will use filepathBuf as a buffer to store the whole command.
	// We ensure it's large enough to hold the whole command plus a '\0'.
	if (!tryReallocIfNecessary((void**)filepathBuf, filepathBufLen, request->filepathLength + CMD_TEMPLATE_1_LEN + CMD_TEMPLATE_2_LEN + 1)) {
		fprintf(stderr, "[Worker] Error: Failed to alloc for cmd length %u.\n", request->filepathLength);
		return 0;
	}
	
	// We use this buffer to form the minisat execution command.
	// This is composed of the concatenation of the strings CMD_TEMPLATE_1,
	// the file path, and CMD_TEMPLATE_2.
	char* buf = *filepathBuf;
	
	// We move the whole filepath ahead in the buffer to make space for CMD_TEMPLATE_1.
	for (int i=request->filepathLength-1; i>=0; i--)
		buf[i + CMD_TEMPLATE_1_LEN] = buf[i];
	
	// We copy CMD_TEMPLATE_1 into the beginning of the buffer.
	for (int i=0; i<CMD_TEMPLATE_1_LEN; i++)
		buf[i] = CMD_TEMPLATE_1[i];
	
	// We copy CMD_TEMPLATE_2 to the rest of the buffer.
	strcpy(buf + request->filepathLength + CMD_TEMPLATE_1_LEN, CMD_TEMPLATE_2);

	FILE* f = popen(buf, "r");
	
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
			exitCode = EXIT_CODE_NOT_ENOUGH_MEMORY;
			break;
		}
		
		result.taskId = request.taskId;
		writeFull(STDOUT_FILENO, &result, sizeof(TWorkerResult));
	}
	
	free(buf);
	
	exit(exitCode);
}