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

#include "./../shared/shmHandler.h"

DataPtr resourceInit(int shmSize, char* shmPath);

DataPtr resourceInit(int shmSize, char* shmPath) {

    Data data;
    DataPtr dataPtr;

    // This practice was recommended in class to avoid issues when the program was interrupted and relaunched (not beeing
	// able to correctly close and clean shmem)
	shm_unlink(data.shmPath);
	shm_unlink(data.mtxPath);
	shm_unlink(data.allPath);

    // Initialize semaphores
    if(!sem_init(&dataPtr->semCanWrite, 1, 1)) {
        perror("sem_init failed");
	    exit(EXIT_FAILURE);
    }
    if(!sem_init(&dataPtr->semCanRead, 1, 0)) {
        perror("sem_init failed");
	    exit(EXIT_FAILURE);
    }

    // Open the shared memory object
	if((data.shmFDes = shm_open(shmPath, O_CREAT | O_RDWR | O_EXCL, S_IWUSR | S_IRUSR)) == -1) {
		perror("shm_open failed");
		exit(EXIT_FAILURE);
	}

	// Preallocate a shared memory area
	if(ftruncate(data.shmFDes, shmSize + sizeof(Data)) == -1) {
		perror("ftruncate failed");
    	exit(EXIT_FAILURE);
	}
	if((data.shmStart = mmap(NULL, shmSize + sizeof(Data), PROT_WRITE | PROT_READ, MAP_SHARED, shmFDes, 0)) == (caddr_t) -1) {
		perror("mmap failed");
    	exit(EXIT_FAILURE);
	}
	
	data.shmSize = shmSize;
	data.dataBuffer = shmPtr + sizeof(Data);
	data.dataBufferSize = shmSize - sizeof(Data);

    if(shmat(data->shmFDes,0,0)) {
		perror("shmat failed");
    	exit(EXIT_FAILURE);
	}
	
	return dataPtr;
}

void resourceUnlink(DataPtr dataPtr) {

	sem_destroy(&dataPtr->semCanRead);
	sem_destroy(&dataPtr->semCanWrite);
	munmap(dataPtr->shmStart, dataPtr->shmSize + sizeof(Data));
	close(dataPtr->shmFDes);
	shm_unlink(dataPtr->shmPath);

}