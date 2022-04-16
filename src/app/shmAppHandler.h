#ifndef _SHMAPPHANDLER_H_
#define _SHMAPPHANDLER_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "./../shared/shmHandler.h"
#include "communication.h"

void resourceInit(char* shmPath, size_t shmSize, TSharedMem* ptrInfoSave);
void resourceUnlink(void* shmStart, TSharedMem* ptrInfoSave);
void outputToShm(const TSharedMem* ptrInfo, unsigned int workerId, const TWorkerResult* result, const char* filepath);

#endif