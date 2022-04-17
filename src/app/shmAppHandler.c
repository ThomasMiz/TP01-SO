// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <string.h>
#include "shmAppHandler.h"
#include "./../shared/satResult.h"
#include "./../shared/memhelper.h"
#include "./../shared/constants.h"

int resourceInit(const char* shmName, size_t shmSize, TSharedMem* ptrInfoSave) {
	// This practice was recommended in class to avoid issues when the program was
	// interrupted and relaunched (not beeing able to correctly close and clean shmem)
	/* if (!shm_unlink(shmName)) {
		fprintf(stderr, "[Master] Warning: previously opened shared memory named \"%s\" was closed.\n", shmName);
	}*/

	// Open the shared memory object
	int shmFDes = shm_open(shmName, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);
	if (shmFDes < 0) {
		perror("[Master] Failed to shm_open");
		return 0;
	}

	// Preallocate a shared memory area
	if (ftruncate(shmFDes, shmSize) == -1) {
		perror("[Master] Failed to ftruncate");
		close(shmFDes);
		shm_unlink(shmName);
    	return 0;
	}
	
	void* shmStart = mmap(NULL, shmSize, PROT_WRITE | PROT_READ, MAP_SHARED, shmFDes, 0);
	if (shmStart == MAP_FAILED) {
		perror("[Master] Failed to mmap");
		close(shmFDes);
		shm_unlink(shmName);
    	return 0;
	}
	
	TSharedMemContext* sharedMemContext = shmStart;

    // Initialize semaphores
    if (sem_init(&sharedMemContext->semCanWrite, 1, 0)) {
        perror("[Master] Failed to sem_init semaphore 1");
		munmap(shmStart, shmSize);
		close(shmFDes);
		shm_unlink(shmName);
	    return 0;
    }
	
    if (sem_init(&sharedMemContext->semCanRead, 1, 0)) {
        perror("[Master] Failed to sem_init semaphore 2");
	    sem_destroy(&sharedMemContext->semCanWrite);
		munmap(shmStart, shmSize);
		close(shmFDes);
		shm_unlink(shmName);
		return 0;
    }
	
	ptrInfoSave->shmStart = shmStart;
	ptrInfoSave->shmSize = shmSize;
	ptrInfoSave->shmName = shmName;
	ptrInfoSave->shmFDes = shmFDes;
	ptrInfoSave->dataBuffer = shmStart + sizeof(TSharedMemContext);
	ptrInfoSave->dataBufferSize = shmSize - sizeof(TSharedMemContext);
	return 1;
}

void resourceUnlink(TSharedMem* ptrInfo) {
	TSharedMemContext* sharedMemContext = ptrInfo->shmStart;

	sem_destroy(&sharedMemContext->semCanRead);
	sem_destroy(&sharedMemContext->semCanWrite);
	munmap(ptrInfo->shmStart, ptrInfo->shmSize);
	close(ptrInfo->shmFDes);
	shm_unlink(ptrInfo->shmName);
}

void outputToShm(const TSharedMem* ptrInfo, unsigned int workerId, const TWorkerResult* result, const char* filepath) {
	TSharedMemContext* sharedMemContext = ptrInfo->shmStart;
	
	TPackage package;
	package.filepathLen = strlen(filepath);
	package.status = result->status;
	package.cantidadClausulas = result->cantidadClausulas;
	package.cantidadVariables = result->cantidadVariables;
	package.timeSeconds = result->timeSeconds;
	package.workerId = workerId;
	
	size_t freeBuffSize = ptrInfo->dataBufferSize - sizeof(TPackage);
	
	if (freeBuffSize < package.filepathLen)
		package.filepathLen = freeBuffSize;
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[Master] Writting to shm: \"%s\" (len %u), %u, %u, %s, %f, %u \n", filepath, package.filepathLen, package.cantidadClausulas, package.cantidadVariables,
			satResultToString(package.status), package.timeSeconds, package.workerId);
#endif

	sem_wait_nointr(&sharedMemContext->semCanWrite);
	memcpy(ptrInfo->dataBuffer, &package, sizeof(TPackage));
	memcpy(ptrInfo->dataBuffer + sizeof(TPackage), filepath, package.filepathLen);
	sem_post(&sharedMemContext->semCanRead);
}