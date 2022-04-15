// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <string.h>
#include "shmViewHandler.h"

void resourceOpen(char* shmName, size_t shmSize, TSharedMem* ptrInfoSave) {

    // Open the shared memory object
	int shmFDes = shm_open(shmName, O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);
	if(shmFDes == -1) {
		perror("shm_open failed");
		exit(EXIT_FAILURE);
	}

	void* shmStart = mmap(NULL, shmSize + sizeof(TSharedMemContext), PROT_WRITE | PROT_READ, MAP_SHARED, ptrInfoSave->shmFDes, 0);
	if(shmStart == MAP_FAILED) {
		perror("mmap failed");
    	exit(EXIT_FAILURE);
	}
	
	ptrInfoSave->shmStart = shmStart;
	ptrInfoSave->shmSize = shmSize;
	ptrInfoSave->shmName = shmName;
	ptrInfoSave->shmFDes = shmFDes;
	ptrInfoSave->dataBuffer = shmStart + sizeof(TSharedMemContext);
	ptrInfoSave->dataBufferSize = shmSize - sizeof(TSharedMemContext);
	
}

void resourceClose(TSharedMem* ptrInfo) {

	TSharedMemContext* sharedMemContext = ptrInfo->shmStart;
	
	sem_close(&sharedMemContext->semCanWrite);
	sem_close(&sharedMemContext->semCanRead);
	munmap(ptrInfo->shmStart, ptrInfo->shmSize + sizeof(TSharedMemContext));
	close(ptrInfo->shmFDes);

}

