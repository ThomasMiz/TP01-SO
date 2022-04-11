#ifndef _SHMHANDLER_H_
#define _SHMHANDLER_H_

#include <semaphore.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

/** Gets some string. */
const char* getSomeSharedString();

typedef struct Resources *ResourcesPtr;

ResourcesPtr resourceInit(int shmSize, char *shmPäth, char *mtxPath, char *allPath);
ResourcesPtr resourcesOpen(int shmSize, char *shmPath, char *mtxPath, char *allPath);

void resourcesClose(ResourcesPtr res);
void resourcesUnlink(ResourcesPtr res);

#endif