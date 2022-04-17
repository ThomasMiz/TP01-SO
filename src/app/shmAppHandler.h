#ifndef _SHM_APP_HANDLER_H_
#define _SHM_APP_HANDLER_H_

#include <stdlib.h>
#include "communication.h"
#include "./../shared/shmHandler.h"

int resourceInit(const char* shmName, size_t shmSize, TSharedMem* ptrInfoSave);

void resourceUnlink(TSharedMem* ptrInfo);

void outputToShm(const TSharedMem* ptrInfo, unsigned int workerId, const TWorkerResult* result, const char* filepath);

#endif