#ifndef _SHMHANDLER_H_
#define _SHMHANDLER_H_

#include <semaphore.h>

#define SHM_SIZE 4096 //This must be bigger than sizeof(TSharedMemContext)

typedef struct {
    sem_t semCanRead;
    sem_t semCanWrite;
	size_t bytesSent;
} TSharedMemContext;

typedef struct {
    void* shmStart;
	size_t shmSize;
	void* dataBuffer;
	size_t dataBufferSize;
	char* shmPath;
	int shmFDes;
} TSharedMem;

#endif