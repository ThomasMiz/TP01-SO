#ifndef _SHMHANDLER_H_
#define _SHMHANDLER_H_

typedef struct {
	int shmFDes;
    void* shmStart;
	int shmSize;
    char* shmPath;
	void* dataBuffer;
	size_t dataBufferSize;
	
    sem_t semCanRead;
    sem_t semCanWait;
} Data;

typedef struct Data* DataPtr;

#endif