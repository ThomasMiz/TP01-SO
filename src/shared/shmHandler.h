#ifndef _SHMHANDLER_H_
#define _SHMHANDLER_H_

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

typedef struct {
	int shmFDes;
    void* shmStart;

    char* shmPath;
    sem_t* mtxPath;
    sem_t* allPath;
} Data;

typedef struct Data* DataPtr;

/** Gets some string. */
const char* getSomeSharedString();


ResourcesPtr resourceInit(int shmSize, char *shmPäth, char *mtxPath, char *allPath);    // Used by app
ResourcesPtr resourcesOpen(int shmSize, char *shmPath, char *mtxPath, char *allPath);   // Used by view

void resourcesUnlink(ResourcesPtr res);
void resourcesClose(ResourcesPtr res);

#endif