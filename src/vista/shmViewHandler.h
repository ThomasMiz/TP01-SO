#ifndef _SHMVIEWHANDLER_H_
#define _SHMVIEWHANDLER_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "./../shared/shmHandler.h"

void resourceOpen(char* shmName, size_t shmSize, TSharedMem* ptrInfoSave);
void resourceClose(TSharedMem* ptrInfoSave);
void readShm(TSharedMem* ptrInfo, TPackage* destination);

#endif