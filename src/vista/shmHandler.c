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

DataPtr resourceOpen(void* shmStart);

DataPtr resourceOpen(void* shmStart) {

    DataPtr dataPtr = (Data)* shmStart;

    // Open the shared memory object
	if((dataPtr->shmFDes = shm_open(dataPtr->shmPath, O_RDWR | O_EXCL, S_IWUSR | S_IRUSR)) == -1) {
		perror("shm_open failed");
		exit(EXIT_FAILURE);
	}


	if((dataPtr->shmStart = mmap(NULL, shmSize + sizeof(Data), PROT_WRITE | PROT_READ, MAP_SHARED, dataPtr->shmFDes, 0)) == (caddr_t) -1) {
		perror("mmap failed");
    	exit(EXIT_FAILURE);
	}
	
	return DataPtr;

}

void resourcesClose(DataPtr dataPtr) {

	sem_close(&dataPtr->semCanWrite);
	sem_close(&dataPtr->semCanRead);
	munmap(dataPtr->shmStart, dataPtr->shmSize + sizeof(Data));
	close(dataPtr->shmFDes);

}

