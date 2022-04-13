#ifndef _SHMHANDLER_H_
#define _SHMHANDLER_H_

#include <semaphore.h>

typedef struct {
    sem_t semCanRead;
    sem_t semCanWrite;
	size_t bytesSent;
} TSharedMemContext;

typedef struct {
    void* shmStart;
	size_t shmSize;
	char* shmName;
	int shmFDes;
	void* dataBuffer;
	size_t dataBufferSize;
} TSharedMem;

#endif