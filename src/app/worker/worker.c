#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "workerHelper.h"
#include "./../communication.h"
#include "./../../shared/constants.h"
#include "./../../shared/memhelper.h"

static const char* cmdTemplate = "minisat \"%s\" | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"";
static size_t cmdTemplateLength;

static void runMinisat(const char* filepath, const TWorkerRequest* request, TWorkerResult* result) {
	size_t cmdSize = cmdTemplateLength + request->filepathLength;
	char* cmd = mallocOrExit(cmdSize + 1);
	sprintf(cmd, cmdTemplate, filepath);
	FILE* f = popen(cmd, "r");
	free(cmd);
	
	interpretMinisatOutput(f, result);
	
	pclose(f);
}

int main(int argc, const char* argv[]) {
	cmdTemplateLength = strlen(cmdTemplate);
	
	TWorkerRequest request;
	TWorkerResult result;
	char* filepath;
	while (readWorkerRequest(STDIN_FILENO, &request, &filepath)) {
		runMinisat(filepath, &request, &result);
		free(filepath);
		
		result.taskId = request.taskId;
		writeFull(STDOUT_FILENO, &result, sizeof(TWorkerResult));
	}
	
	exit(0);
}