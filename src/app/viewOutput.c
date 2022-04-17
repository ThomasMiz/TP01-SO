// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include "viewOutput.h"
#include "app.h"
#include "shmAppHandler.h"
#include "./../shared/memhelper.h"
#include "./../shared/constants.h"

/**
 * Generates a name to use for the shared memory.
 *
 * Returns a pointer to an allocated string which the
 * caller is responsible for freeing, or NULL on error.
 */
static char* generateShmName() {
	// Used for the seeding the rand_r.
	unsigned int seed = (unsigned int)(time(NULL) ^ getpid());
	
	char* s;
	if (!tryMalloc((void**)&s, SHM_NAME_LENGTH + 2))
		return NULL;
	
	s[0] = '/';
	s[SHM_NAME_LENGTH + 1] = '\0';
	
	// The name will consist of interleaved vowels and consonants.
	for (int i=0; i<SHM_NAME_LENGTH; i++) {
		if (i % 2 == 0)
			s[i + 1] = "BCDFGHJKLMNPQRTSVWXYZ"[rand_r(&seed) % 21];
		else
			s[i + 1] = "AEIOU"[rand_r(&seed) % 5];
	}
	
	return s;
}

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

	char* shmName = generateShmName();

	// Creation of shared mem and semaphores.
	if (!resourceInit(shmName, MAX_SHM_SIZE, &appContext->shmInfo)) {
		appContext->shmInfo.shmStart = NULL;
		free(shmName);
		return 0;
	}
	
	TSharedMemContext* sharedMemContext = appContext->shmInfo.shmStart;
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Printing shm signature.\n");
#endif
	
	// Print the shared memory's name and size to STDOUT.
	printf("%s:%lu\n", appContext->shmInfo.shmName, (unsigned long)appContext->shmInfo.shmSize);
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
		free(shmName);
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
		free((void*)appContext->shmInfo.shmName);
		appContext->shmInfo.shmStart = NULL;
	}
}