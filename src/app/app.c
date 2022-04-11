#include <stdio.h>
#include "app.h"
#include "appHelper.h"
#include "workerManagerADT.h"
#include "./../shared/constants.h"


/** Callback function for whenever a worker returns a result. */
static void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg);

/** Callack function for whenever a worker closes.*/
static void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg);


int main(int argc, const char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "[Master] Error: no input files.\n");
		return EXIT_CODE_NOT_ENOUGH_PARAMS;
	}
	
	// We create a context struct and fill it up with initial data.
	TAppContext appContext;
	appContext.files = &argv[1];
	appContext.fileCount = argc - 1;
	appContext.filesSent = 0;
	appContext.resultsReceived = 0;
	
	// Create a new workerManager with the desired amount of workers.
	workerManagerADT workerManager = newWorkerManager(decideWorkerCount(appContext.fileCount));
	fprintf(stderr, "Number of workers: %u\n", getWorkerCount(workerManager));// TODO: remove debug print
	
	// Set the callbacks on the workerManager. The parameter for the
	// callbacks is a pointer to our app context struct. This is memory
	// safe even though appContext is on the stack because the lifetime
	// of the workerManager is entirely within this function, so the
	// TAppContext is guaranteed to still be on the stack.
	setWorkerResultCallback(workerManager, &onWorkerResult, &appContext);
	setWorkerClosedCallback(workerManager, &onWorkerClosed, &appContext);
	
	// Send initial tasks to the workers. This function also decides how many per worker.
	sendInitialWorkerTasks(workerManager, &appContext);
	
	closeRemainingWorkers(workerManager); // TODO: you know what to do here, Thomas.
	pollUntilFinished(workerManager);
	
	// We free up all resources used by the workerManager.
	freeWorkerManager(workerManager);
	
	fprintf(stderr, "[Master] Master process closing.\n"); // TODO: remove debug prints
	return 0;
}

static void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg) {
	fprintf(stderr, "[Master] Worker %u returned result: taskId=%u, cantidadClausulas=%u, cantidadVariables=%u.\n", workerId, result->taskId, result->cantidadClausulas, result->cantidadVariables);
}

static void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg) {
	fprintf(stderr, "[Master] Worker %u closed.\n", workerId);
}
