#ifndef _SHMHANDLER_H_
#define _SHMHANDLER_H_

#include <semaphore.h>
#include "./../app/communication.h" //Because we need to know the struct of result in view and app

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

typedef struct {
	const char* filepath;
	const TWorkerResult* result;
	unsigned int workerId;
	const char* status;
} TPackage;

#endif