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
#include "shmAppHandler.h"

void resourceInit(char* shmName, size_t shmSize, TSharedMem* ptrInfoSave) {

    // This practice was recommended in class to avoid issues when the program was interrupted and relaunched (not beeing
	// able to correctly close and clean shmem)
	shm_unlink(shmName);

	// Open the shared memory object
	int shmFDes = shm_open(shmName, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);
	if(shmFDes < 0) {
		perror("shm_open failed");
		exit(EXIT_FAILURE);
	}

	// Preallocate a shared memory area
	if(ftruncate(shmFDes, shmSize + sizeof(TSharedMemContext)) == -1) {
		perror("ftruncate failed");
    	exit(EXIT_FAILURE);
	}
	void* shmStart = mmap(NULL, shmSize + sizeof(TSharedMemContext), PROT_WRITE | PROT_READ, MAP_SHARED, shmFDes, 0);
	if(shmStart == MAP_FAILED) {
		perror("mmap failed");
    	exit(EXIT_FAILURE);
	}
	
	ptrInfoSave->shmStart = shmStart;
	ptrInfoSave->shmSize = shmSize;
	ptrInfoSave->shmName= shmName;
	ptrInfoSave->shmFDes = shmFDes;
	ptrInfoSave->dataBuffer = shmStart + sizeof(TSharedMemContext);
	ptrInfoSave->dataBufferSize = shmSize - sizeof(TSharedMemContext);
	
	TSharedMemContext* sharedMemContext = shmStart;

    // Initialize semaphores
    if(sem_init(&sharedMemContext->semCanWrite, 1, 1)) {
        perror("sem_init failed");
	    exit(EXIT_FAILURE);
    }
    if(sem_init(&sharedMemContext->semCanRead, 1, 0)) {
        perror("sem_init failed");
	    exit(EXIT_FAILURE);
    }
	
	// to stdout shared memory path
	write(STDOUT_FILENO, shmName, strlen(shmName) + 1);
}

void resourceUnlink(void* shmStart, TSharedMem* ptrInfo) {
	
	TSharedMemContext* sharedMemContext = shmStart;

	sem_destroy(&sharedMemContext->semCanRead);
	sem_destroy(&sharedMemContext->semCanWrite);
	munmap(ptrInfo->shmStart, ptrInfo->shmSize + sizeof(TSharedMemContext));
	close(ptrInfo->shmFDes);
	shm_unlink(ptrInfo->shmName);

}