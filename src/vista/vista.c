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
	size_t shmSize = 0;
	
	int matches;
	if (argc == 2)
		matches = sscanf(argv[1], "%[/a-zA-Z]:%lu", shmName, &shmSize);
	else
		matches = fscanf(stdin, "%[/a-zA-Z]:%lu\n", shmName, &shmSize);
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[View] Connecting to %s with size %lu.\n", shmName, shmSize);
#endif

	TSharedMem ptrInfo;
	resourceOpen(shmName, shmSize, &ptrInfo);
	TSharedMemContext* sharedMemContext = ptrInfo.shmStart;
	
	TPackage privPackage;
	char* privStr = NULL;
	size_t privStrMaxLen = 0;
	
	fprintf(stderr, "Posting semCanWrite");
	if (sem_post(&sharedMemContext->semCanRead)) {
		perror(NULL);
		exit(-1);
	}
	
	fprintf(stderr, "La willy willy");
	while (readShm(&ptrInfo, &privPackage, &privStr, &privStrMaxLen)) {
		printf("\"%s\", %u, %u, %s, %f, %u \n", privStr, privPackage.cantidadClausulas, privPackage.cantidadVariables,
			satResultToString(privPackage.status), privPackage.timeSeconds, privPackage.workerId);
	}
	
	resourceClose(&ptrInfo);

	return 0;
}