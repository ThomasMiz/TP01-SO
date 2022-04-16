// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "shmViewHandler.h"
#include "./../shared/constants.h"
#include "./../shared/shmHandler.h"

int main(int argc, const char* argv[]) {		//Recibe nombre:size

	if (argc > 2) {
		fprintf(stderr, "Too many arguments: %d \n", argc);
        exit(EXIT_FAILURE);
	}

	char shmName[11];
	size_t shmSize;
	
	if (argc < 2) {
		fscanf(stdin, "%10s:%lu\n", shmName, &shmSize);
	} else if (argc == 2) {
		scanf(argv[1], "%10s:%lu", shmName, &shmSize);
	}
		// if(pToken == NULL) {
			// fprintf(stderr, "Wrong input, shmSize arg NULL \n");
			// exit(EXIT_FAILURE);
		// }
		// shmSize = atoi(pToken);
		// if (shmSize > MAX_SHM_SIZE) {
			// fprintf(stderr, "Shared memory space requested too big \n");
			// exit(EXIT_FAILURE);
		// }
	
	TSharedMem ptrInfo;
	resourceOpen(shmName, shmSize, &ptrInfo);
	TSharedMemContext* sharedMemContext = ptrInfo.shmStart;
	
	TPackage privPackage;
	char* privStr = NULL;
	size_t privStrMaxLen = 0;
	
	sem_post(&sharedMemContext->semCanWrite);
	
	while (1) {
		
		if(!readShm(&ptrInfo, &privPackage, &privStr, &privStrMaxLen)) {
			break;
		}
		
		printf("\"%s\", %u, %u, %s, %f, %u \n", privStr, privPackage.cantidadClausulas, privPackage.cantidadVariables,
			satResultToString(privPackage.status), privPackage.timeSeconds, privPackage.workerId);
	}
	
	resourceClose(&ptrInfo);

	return 0;
}