#ifndef _SHM_APP_HANDLER_H_
#define _SHM_APP_HANDLER_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "./../shared/shmHandler.h"
#include "communication.h"

int resourceInit(const char* shmName, size_t shmSize, TSharedMem* ptrInfoSave);
void resourceUnlink(TSharedMem* ptrInfo);
void outputToShm(const TSharedMem* ptrInfo, unsigned int workerId, const TWorkerResult* result, const char* filepath);

#endif