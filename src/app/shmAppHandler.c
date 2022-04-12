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

void resourceInit(char* shmPath, TSharedMem* ptrInfoSave) {

    // This practice was recommended in class to avoid issues when the program was interrupted and relaunched (not beeing
	// able to correctly close and clean shmem)
	shm_unlink(shmPath);

	// Open the shared memory object
	int shmFDes = shm_open(shmPath, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR);
	if(shmFDes < 0) {
		perror("shm_open failed");
		exit(EXIT_FAILURE);
	}

	// Preallocate a shared memory area
	if(ftruncate(shmFDes, SHM_SIZE + sizeof(TSharedMemContext)) == -1) {
		perror("ftruncate failed");
    	exit(EXIT_FAILURE);
	}
	void* shmStart = mmap(NULL, SHM_SIZE + sizeof(TSharedMemContext), PROT_WRITE | PROT_READ, MAP_SHARED, shmFDes, 0);
	if(shmStart == MAP_FAILED) {
		perror("mmap failed");
    	exit(EXIT_FAILURE);
	}
	
	TSharedMem sharedMem;
	sharedMem.shmStart = shmStart;
	sharedMem.shmSize = SHM_SIZE;
	sharedMem.shmPath=shmPath;
	sharedMem.shmFDes = shmFDes;
	sharedMem.dataBuffer = shmStart + sizeof(TSharedMemContext);
	sharedMem.dataBufferSize = SHM_SIZE - sizeof(TSharedMemContext);
	*ptrInfoSave = sharedMem;	//al ptr (UP_CHANGES)
	
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
	write(STDOUT_FILENO, shmPath, strlen(shmPath) + 1);
}

void resourceUnlink(void* shmStart, TSharedMem* ptrInfoSave) {
	
	TSharedMemContext* sharedMemContext = shmStart;

	sem_destroy(&sharedMemContext->semCanRead);
	sem_destroy(&sharedMemContext->semCanWrite);
	munmap(ptrInfoSave->shmStart, ptrInfoSave->shmSize + sizeof(TSharedMemContext));
	close(ptrInfoSave->shmFDes);
	shm_unlink(ptrInfoSave->shmPath);

}