// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include "fileOutput.h"
#include "./../shared/constants.h"

int fileOutputBegin(TAppContext* appContext) {
	// This might return NULL. In which case, we don't output.
	FILE* file = fopen(RESULT_OUTPUT_FILE, "w");
	appContext->resultOutputFile = file;
	
	// We print an error if the file couldn't be opened.
	if (!file) {
		perror("[Master] Error: Failed to create file %s for writting output");
		return 0;
	}
	
	fprintf(file, "archivo, clausulas, variables, resultado, tiempo, idWorker\n");
	return 1;
}

void fileOutputResult(TAppContext* appContext, unsigned int workerId, const TWorkerResult* result, const char* filepath) {
	if (appContext->resultOutputFile) {
		fprintf(appContext->resultOutputFile, "\"%s\", %u, %u, %s, %f, %u\n", filepath, result->cantidadClausulas,
			result->cantidadVariables, satResultToString(result->status), result->timeSeconds, workerId);
	}
}

void fileOutputEnd(TAppContext* appContext) {
	if (appContext->resultOutputFile) {
		fflush(appContext->resultOutputFile);
		fclose(appContext->resultOutputFile);
		appContext->resultOutputFile = NULL;
	}
}