#include <stdio.h>
#include <unistd.h>
#include "./../shared/constants.h"
#include "workerManagerADT.h"


/** Callback function for whenever a worker returns a result. */
static void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg);

/** Callack function for whenever a worker closes.*/
static void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg);


/** Stores context information about the app process. */
typedef struct {
	/** An array with the files to process, of length fileCount. */
	const const char** files;
	
	/** The total amount of files the app process was given. */
	unsigned int fileCount;
	
	/** The amount of files that have already been sent to workers,
	 * but not necessarily gotten results for.
	 */
	unsigned int filesSent;
	
	/** The amount of results received from workers. */
	unsigned int resultsReceived;
} TAppContext;


/**
 * Decides the amount of workers to spawn based on constants and
 * the configuration of the running system.
 *
 * Will always return a value of at least 1, unless fileCount is 0.
 */
static unsigned int decideWorkerCount(unsigned int fileCount) {
	long systemProcessors = sysconf(_SC_NPROCESSORS_ONLN);
	
#if LEAVE_ONE_PROCESSOR == 1
	unsigned int workerLimit = systemProcessors <= 1 ? 1 : (systemProcessors - 1);
#else
	unsigned int workerLimit = systemProcessors <= 1 ? 1 : systemProcessors;
#endif

	if (fileCount > workerLimit)
		fileCount = workerLimit;
	
#if MAX_WORKERS > 0
	if (fileCount > MAX_WORKERS)
		fileCount = MAX_WORKERS;
#endif
	
	return fileCount;
}

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
	
	for (int i=1; i<argc; i++)
		sendWorkerTask(workerManager, (i - 1) % getWorkerCount(workerManager), i - 1, argv[i]);
	
	closeRemainingWorkers(workerManager);
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
