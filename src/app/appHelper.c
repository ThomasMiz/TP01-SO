// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <unistd.h>
#include "appHelper.h"
#include "./../shared/constants.h"

unsigned int decideWorkerCount(unsigned int fileCount) {
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

void sendInitialWorkerTasks(workerManagerADT workerManager, TAppContext* appContext) {
	// We will send 1/10th of files rounded down.
	// We will however always send at least one initial file to each worker.
	
	if (appContext->filesSent) {
		fprintf(stderr, "[Master] Error: sendInitialWorkerTasks() called with appContext that has already sent files.\n");
		return;
	}
	
	unsigned int workerCount = getRemainingWorkerCount(workerManager);
	
	if (workerCount > appContext->fileCount) {
		fprintf(stderr, "[Master] Warning: sendInitialWorkerTasks() found that more workers were spawned than files requested.\n");
		workerCount = appContext->fileCount;
	}
	
	unsigned int filesPerWorker = (appContext->fileCount / 10) / workerCount;
	if (!filesPerWorker)
		filesPerWorker = 1;
	
	unsigned int taskId = 0;
	for (int f=0; f<filesPerWorker; f++) {
		for (int workerId=0; workerId<workerCount; workerId++) {
			sendWorkerTask(workerManager, workerId, taskId, appContext->files[taskId]);
			taskId++;
		}
	}
	
	appContext->filesSent = taskId;
}