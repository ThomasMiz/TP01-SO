#ifndef _SHM_VIEW_HANDLER_H_
#define _SHM_VIEW_HANDLER_H_

#include <stdlib.h>
#include "./../shared/shmHandler.h"

int resourceOpen(char* shmName, size_t shmSize, TSharedMem* ptrInfoSave);

void resourceClose(TSharedMem* ptrInfoSave);

int readShm(TSharedMem* ptrInfo, TPackage* destination, char** privStr, size_t* privStrMaxLen);

#endif