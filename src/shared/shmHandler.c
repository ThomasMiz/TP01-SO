#include "shmHandler.h"

const char* getSomeSharedString() {
	return "This string came from shared code!";
}

struct Resources {

	char *shmStart;
	int shmSize, shmFDes;
	sem_t *mtxSemp, *allSemp;
	char *shmPath, *mtxPath, *allPath;

}

ResourcesPtr resourceInit(int shmSize, char *shmPäth, char *mtxPath, char *allPath) {

	ResourcesPtr res = (ResourcesPtr)malloc(sizeof(struct Resources));
	res -> shmSize = shmSize;
	res -> shmPath = shmPath;
	res -> mtxPath = mtxPath;
	res -> allPath = allPath;

	// This practice was recommended in class to avoid issues when the program was interrupted and relaunched (not beeing
	// able to correctly close and clean shmem)
	shm_unlink(res -> shmPath);
	shm_unlink(res -> mtxPath);
	shm_unlink(res -> allPath);

	// Open semaphores with combination of flags that ensure to create a semaphore if it does not already exist
	// and sem_open should fail if the semaphore already exists
	if((res -> mtxSemp = sem_open(mtxPath, O_CREAT | O_EXCL, 0660, 1)) == -1) {
		perror("sem_open() failed");
		exit(EXIT_FAILURE);
	}
	if((res -> allSemp = sem_open(allPath,  O_CREAT | O_EXCL, 0660, 0)) == -1) {
		perror("sem_open() failed");
		exit(EXIT_FAILURE);
	}

	// Open the shared memory object
	if((res -> shmFDes = shm_open(shmPath, O_CREAT | O_RDWR | O_EXCL | O_TRUNC, S_IWUSR | S_IRUSR)) == -1) {
		perror("shm_open() failed");
		exit(EXIT_FAILURE);
	}

	// Preallocate a shared memory area
	if(ftruncate(res -> shmFdes, res -> shmSize + sizeof(long)) == -1) {
		perror("ftruncate failure");
    	exit();
	}
	if((res -> shmBase = mmap(NULL, res -> shmSize + sizeof(long), PROT_WRITE | PROT_READ, MAP_SHARED, res -> shmFDes, 0)) == (caddr_t) -1) {
		perror("mmap failure");
    	exit();
	}

    return res;
}

// Some flags change when only opening eg the O_CREAT in sem_open()
ResourcesPtr resourcesOpen(int shmSize, char *shmPath, char *mtxPath, char *allPath) {

	ResourcesPtr res = (ResourcesPtr)malloc(sizeof(struct Resources));
	res -> shmSize = shmSize;
	res -> shmPath = shmPath;
	res -> mtxPath = mtxPath;
	res -> allPath = allPath;

	// Semaphore open
	if((res -> mtxSemp = sem_open(mtxPath, 0, 0660, 1)) == -1) {
		perror("sem_open() failed");
		exit(EXIT_FAILURE);
	}
	if((res -> allSemp = sem_open(allPath,  0, 0660, 0)) == -1) {
		perror("sem_open() failed");
		exit(EXIT_FAILURE);
	}

	// Shared memory open
	if((res -> shmFDes = shm_open(shmPath, O_RDWR | O_EXCL, S_IWUSR | S_IRUSR)) == -1) {
		perror("shm_open() failed");
		exit(EXIT_FAILURE);
	}

	// Shared memory mapping
	if((res -> shmStart =  mmap(NULL, res -> shmSize + sizeof(long), PROT_WRITE | PROT_READ, MAP_SHARED, res -> shmFDes, 0)) == (caddr_t) -1) {
		perror("mmap failure");
    	exit();
	}

	return res;

}

void resourcesClose(ResourcesPtr res) {

	sem_close(res -> mtxSemp);
	sem_close(res -> allSemp);
	munmap(res -> shmStart, res -> shmSize + sizeof(long));
	close(res -> shmFDes);
	free(res);

}

void resourcesUnlink(ResourcesPtr res) {

	munmap(res -> shmStart, res -> shmSize + sizeof(long));
	shm_unlink(res -> shmPath);
	shm_unlink(res -> allPath);
	sem_unlink(res -> mtxSemp);
	sem_unlink(res -> allSemp);
	free(res);

}