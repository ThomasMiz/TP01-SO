// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include "shmViewHandler.h"
#include "./../shared/memhelper.h"
#include "./../shared/constants.h"

int resourceOpen(char* shmName, size_t shmSize, TSharedMem* ptrInfoSave) {

    // Open the shared memory object
	int shmFDes = shm_open(shmName, O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);
	if(shmFDes == -1) {
		perror("[View]: shm_open failed");
		return 0;
	}

	void* shmStart = mmap(NULL, shmSize, PROT_WRITE | PROT_READ, MAP_SHARED, shmFDes, 0);
	if(shmStart == MAP_FAILED) {
		perror("[View] mmap failed");
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

void resourceClose(TSharedMem* ptrInfo) {
	munmap(ptrInfo->shmStart, ptrInfo->shmSize);
	close(ptrInfo->shmFDes);
}

int readShm(TSharedMem* ptrInfo, TPackage* destination, char** privStr, size_t* privStrMaxLen) {

	TSharedMemContext* sharedMemContext = ptrInfo->shmStart;
	
	sem_wait_nointr(&sharedMemContext->semCanRead);
	memcpy(destination, ptrInfo->dataBuffer, sizeof(TPackage));
	if (destination->filepathLen) {
		if (!tryReallocIfNecessary((void**) privStr, privStrMaxLen, destination->filepathLen + 1)) {
			fprintf(stderr, "[View] Failed to alloc for cmd length %u.\n", destination->filepathLen + 1);
			return 0;
		}
		memcpy(*privStr, ptrInfo->dataBuffer + sizeof(TPackage), destination->filepathLen);
	}
	
	sem_post(&sharedMemContext->semCanWrite);
	
	if (destination->filepathLen)
		(*privStr)[destination->filepathLen] = '\0';
	
	return 1;

}