// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>#include <assert.h>#include <semaphore.h>#include <signal.h>#include <errno.h>#include "app.h"#include "appHelper.h"#include "workerManagerADT.h"#include "output.h"#include "shmAppHandler.h"#include "./../shared/constants.h"
/** Callback function for whenever a worker returns a result. */
static void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg);
/** Callack function for whenever a worker closes.*/
static void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg);
int main(int argc, const char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "[Master] Error: no input files.\n");
		return EXIT_CODE_NOT_ENOUGH_PARAMS;	}
	// We create a context struct and fill it up with initial data.	TAppContext appContext;	appContext.files = &argv[1];	appContext.fileCount = argc - 1;	appContext.filesSent = 0;	appContext.resultsReceived = 0;
	
	// Wait time until view responds.
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("clock_gettime"); 
		return EXIT_FAILURE;
	}
	ts.tv_sec += MAX_TIME_WAIT;
	// Creation of shared mem and semaphores.	resourceInit("/myShm", MAX_SHM_SIZE, &appContext.ptrInfo);	appContext.sharedMemContext = appContext.ptrInfo.shmStart;	printf("%s:%lu\n", appContext.ptrInfo.shmName, appContext.ptrInfo.shmSize);
	
	// Wait until the view ack, or a timeout expires.
	int s;
	while ((s = sem_timedwait(&appContext.sharedMemContext->semCanRead, &ts)) == -1 && errno == EINTR);
	// Check what happened
	if (s == -1) {		if (errno == ETIMEDOUT)			printf("View not connected.\n");
		else
			perror("sem_timedwait");
	} else {
		printf("View connected!\n");
	}

	// Tell output.c to prepare for outputting results.
	int hasOutputFile = onOutputBegin(&appContext);
	// Create a new workerManager with the desired amount of workers.
	workerManagerADT workerManager = newWorkerManager(decideWorkerCount(appContext.fileCount));
	if (workerManager == NULL) {
		fprintf(stderr, "[Master] Error: failed to create worker manager. Aborting.\n");
		return 1;
	}
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Number of workers: %u\n", getRemainingWorkerCount(workerManager));
#endif
	// Set the callbacks on the workerManager. The parameter for the
	// callbacks is a pointer to our app context struct. This is memory
	// safe even though appContext is on the stack because the lifetime
	// of the workerManager is entirely within this function, so the
	// TAppContext is guaranteed to still be on the stack.
	setWorkerResultCallback(workerManager, &onWorkerResult, &appContext);
	setWorkerClosedCallback(workerManager, &onWorkerClosed, &appContext);

	// Send initial tasks to the workers. This function also decides how many per worker.
	sendInitialWorkerTasks(workerManager, &appContext);
	// sendInitialWorkerTasks() might have sent all the files (if, for example,
	// there were very few files specified). In which case, we close all workers.
	if (appContext.fileCount == appContext.filesSent)
		closeRemainingWorkers(workerManager);
	// This function will process all the workers and block until they're all done.
	pollUntilFinished(workerManager);
	// We free up all resources used by the workerManager.
	freeWorkerManager(workerManager);
	// Tell output.c that there are no more results to output.
	onOutputEnd(&appContext);
	if (hasOutputFile)
		printf("Done. Results saved in file %s.\n", RESULT_OUTPUT_FILE);
	else
		printf("Done. Results not saved because file %s couldn't be opened.\n", RESULT_OUTPUT_FILE);
	
	return 0;
}
static void onWorkerResult(workerManagerADT sender, unsigned int workerId, const TWorkerResult* result, void* arg) {
	TAppContext* appContext = arg;
	appContext->resultsReceived++;
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Worker %u returned result: taskId=%u, cantidadClausulas=%u, cantidadVariables=%u.\n", workerId, result->taskId, result->cantidadClausulas, result->cantidadVariables);
#endif
	// If there are more files to send and the worker has no pending
	// tasks, we send it a new task.
	if (getWorkerRemainingTasks(sender, workerId) == 0 && appContext->filesSent < appContext->fileCount) {
		unsigned int newTaskId = appContext->filesSent++;
		sendWorkerTask(sender, workerId, newTaskId, appContext->files[newTaskId]);
		// If that was the last file, we close all remaining workers.
		if (appContext->filesSent == appContext->fileCount)
			closeRemainingWorkers(sender);
	}
	
	const char* filepath;
	if (result->taskId < appContext->fileCount) {
		filepath = appContext->files[result->taskId];
	} else {
		fprintf(stderr, "[Master] Error: Worker %u returned result with invalid taskId %u.\n", workerId, result->taskId);
		filepath = "?";
	}
	// Send the result to output.c
	onOutputResult(appContext, workerId, result, filepath);
	
	// Send the result to SHM
	outputToShm(&appContext->ptrInfo, workerId, result, filepath);
}
static void onWorkerClosed(workerManagerADT sender, unsigned int workerId, void* arg) {
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Worker %u closed.\n", workerId);
#endif
}