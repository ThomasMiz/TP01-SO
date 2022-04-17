// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include "app.h"
#include "appHelper.h"
#include "workerManagerADT.h"
#include "fileOutput.h"
#include "viewOutput.h"
#include "shmAppHandler.h"
#include "./../shared/memhelper.h"
#include "./../shared/constants.h"

int viewOutputBegin(TAppContext* appContext) {
	appContext->shmInfo.shmStart = NULL;
	appContext->shmInfo.shmSize = 0;
	appContext->shmInfo.shmName = NULL;
	appContext->shmInfo.shmFDes = -1;
	appContext->shmInfo.dataBuffer = NULL;
	appContext->shmInfo.dataBufferSize = 0;
	
	// Calculate the timespec for the timedwait later.
	struct timespec ts;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("[Master] Error: Failed to clock_gettime"); 
		return 0;
	}
	ts.tv_sec += MAX_TIME_WAIT;
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Initializing shared memory.\n");
#endif

	// Creation of shared mem and semaphores.
	if (!resourceInit("/myShm", MAX_SHM_SIZE, &appContext->shmInfo)) {
		appContext->shmInfo.shmStart = NULL;
		return 0;
	}
	
	TSharedMemContext* sharedMemContext = appContext->shmInfo.shmStart;
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Printing shm signature.\n");
#endif
	
	// Print the shared memory's name and size to STDOUT.
	printf("%s:%lu\n", appContext->shmInfo.shmName, appContext->shmInfo.shmSize);
	fflush(stdout);
	
	// Wait until the view ack, or a timeout expires.
	int s;
	while ((s = sem_timedwait(&sharedMemContext->semCanWrite, &ts)) == -1 && errno == EINTR);

	// Check what happened
	if (s == -1) {
		if (errno == ETIMEDOUT)
			printf("View not connected.\n");
		else
			perror("[Master] Error: Failed to sem_timedwait while waiting for view");
		
		resourceUnlink(&appContext->shmInfo);
		appContext->shmInfo.shmStart = NULL;
		return 0;
	}
	
	// Post on the write semaphore, because view starts waiting to read.
	sem_post(&sharedMemContext->semCanWrite);
	
	printf("View connected!\n");
	return 1;
}

void viewOutputResult(TAppContext* appContext, unsigned int workerId, const TWorkerResult* result, const char* filepath) {
	if (appContext->shmInfo.shmStart != NULL)
		outputToShm(&appContext->shmInfo, workerId, result, filepath);
}

void viewOutputEnd(TAppContext* appContext) {
	if (appContext->shmInfo.shmStart != NULL) {
		TSharedMemContext* sharedMemContext = appContext->shmInfo.shmStart;
		
#if DEBUG_PRINTS == 1
		fprintf(stderr, "[Master] Sending disconnect package to view.\n");
#endif
		// Tell the view there are no more results by sending an empty result.
		sem_wait_nointr(&sharedMemContext->semCanWrite);
		memset(appContext->shmInfo.dataBuffer, 0, sizeof(TPackage));
		sem_post(&sharedMemContext->semCanRead);
		
		// Wait for the view to ack that package.
		sem_wait_nointr(&sharedMemContext->semCanWrite);

#if DEBUG_PRINTS == 1
		fprintf(stderr, "[Master] Received disconnect ack from view. Closing shm.\n");
#endif
		
		// View is disconnected. We can now close the memory.
		resourceUnlink(&appContext->shmInfo);
		appContext->shmInfo.shmStart = NULL;
	}
}