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

void resourceInit(int shmSize, char* shmPath);

void resourceInit(int shmSize, char* shmPath) {

    Data data;
    DataPtr dataPtr;

    // This practice was recommended in class to avoid issues when the program was interrupted and relaunched (not beeing
	// able to correctly close and clean shmem)
	shm_unlink(res -> shmPath);
	shm_unlink(res -> mtxPath);
	shm_unlink(res -> allPath);

    // Initialize semaphores
    if(!sem_init(data.mtxPath, 1, 1)) {
        perror("sem_init failed");
	    exit(EXIT_FAILURE);
    }
    if(!sem_init(data.allPath, 1, 0)) {
        perror("sem_init failed");
	    exit(EXIT_FAILURE);
    }

    // Open the shared memory object
	if((data.shmFDes = shm_open(shmPath, O_CREAT | O_RDWR | O_EXCL | O_TRUNC, S_IWUSR | S_IRUSR)) == -1) {
		perror("shm_open failed");
		exit(EXIT_FAILURE);
	}

	// Preallocate a shared memory area
	if(ftruncate(data.shmFDes, shmSize + sizeof(struct Data)) == -1) {
		perror("ftruncate failed");
    	exit(EXIT_FAILURE);
	}

	if((data.shmStart = mmap(NULL, shmSize + sizeof(struct Data), PROT_WRITE | PROT_READ, MAP_SHARED, shmFDes, 0)) == (caddr_t) -1) {
		perror("mmap failed");
    	exit(EXIT_FAILURE);
	}

    dataPtr = shmat(data->shmFDes,0,0);

}