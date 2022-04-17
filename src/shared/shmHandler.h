#ifndef _SHM_HANDLER_H_
#define _SHM_HANDLER_H_

#include <semaphore.h>
#include "./../shared/satResult.h"

typedef struct {
    sem_t semCanRead;
    sem_t semCanWrite;
} TSharedMemContext;

typedef struct {
    void* shmStart;
	size_t shmSize;
	
	const char* shmName;
	int shmFDes;
	
	void* dataBuffer;
	size_t dataBufferSize;
} TSharedMem;

typedef struct {
	unsigned int filepathLen;
	enum SatResult status;
	unsigned int cantidadClausulas;
	unsigned int cantidadVariables;
	double timeSeconds;
	unsigned int workerId;
} TPackage;

#endif