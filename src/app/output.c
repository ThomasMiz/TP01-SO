// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "output.h"
#include "app.h"
#include "communication.h"
#include "./../shared/constants.h"

void onOutputBegin(TAppContext* appContext) {
	FILE* file = fopen(RESULT_OUTPUT_FILE, "w");
	appContext->resultOutputFile = file;
	
	// We print an error if the file couldn't be opened.
	if (!file) {
		fprintf(stderr, "[Master] Error: Failed to create file %s for writting output: ", RESULT_OUTPUT_FILE);
		perror(NULL);
		return;
	}
	
	fprintf(file, "archivo, clausulas, variables, resultado, tiempoNanos, idWorker\n");
}

void onOutputResult(TAppContext* appContext, unsigned int workerId, const TWorkerResult* result, const char* filepath) {
	if (appContext->resultOutputFile) {
		fprintf(appContext->resultOutputFile, "\"%s\", %u, %u, %s, %lu, %u\n", filepath, result->cantidadClausulas,
			result->cantidadVariables, satResultToString(result->status), result->timeNanoseconds, workerId);
	}
}

void onOutputEnd(TAppContext* appContext) {
	if (appContext->resultOutputFile) {
		fflush(appContext->resultOutputFile);
		fclose(appContext->resultOutputFile);
		appContext->resultOutputFile = NULL;
	}
}