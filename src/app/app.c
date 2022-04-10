#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "constants.h"
#include "workerManagerADT.h"

void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg);
void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg);

int main(int argc, const char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "[Master] Error: no input files.\n");
		return EXIT_CODE_NOT_ENOUGH_PARAMS;
	}
	
	// The amount of workers to spawn will be the amount of files
	// we have to process, clamped to the maximum allowed.
	unsigned int workerCount = argc - 1;
	if (workerCount > MAX_WORKERS)
		workerCount = MAX_WORKERS;
	
	workerManagerADT workerManager = newWorkerManager(workerCount);
	setWorkerResultCallback(workerManager, &onWorkerResult, NULL);
	setWorkerClosedCallback(workerManager, &onWorkerClosed, NULL);
	
	for (int i=1; i<argc; i++)
		sendWorkerTask(workerManager, (i - 1) % workerCount, i - 1, argv[i]);
	
	closeRemainingWorkers(workerManager);
	pollUntilFinished(workerManager);
	
	freeWorkerManager(workerManager);
	
	fprintf(stderr, "[Master] Master process closing.\n");
	return 0;
}

void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg) {
	fprintf(stderr, "[Master] Worker %u returned result: taskId=%u, cantidadClausulas=%u, cantidadVariables=%u.\n", workerId, result->taskId, result->cantidadClausulas, result->cantidadVariables);
}

void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg) {
	fprintf(stderr, "[Master] Worker %u closed.\n", workerId);
}
