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
		printf("Too many arguments: %d \n", argc);
        exit(EXIT_FAILURE);
	}
	
	// Disable buffering the output.
	setvbuf(stdout, NULL, _IONBF, 0);

	char shmName[11];
	size_t shmSize = 0;
	
	int matches;
	if (argc == 2)
		matches = sscanf(argv[1], "%10[/a-zA-Z]:%lu", shmName, &shmSize);
	else {
		matches = fscanf(stdin, "%10[/a-zA-Z]:%lu", shmName, &shmSize);
	}
#if DEBUG_PRINTS == 1
	printf("[View] Connecting to %s with size %lu.\n", shmName, shmSize);
#endif

	TSharedMem ptrInfo;
	resourceOpen(shmName, shmSize, &ptrInfo);
	TSharedMemContext* sharedMemContext = ptrInfo.shmStart;
	
	TPackage privPackage;
	char* privStr = NULL;
	size_t privStrMaxLen = 0;
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[View] Ack by posting to semCanRead.\n");
#endif

	if (sem_post(&sharedMemContext->semCanWrite)) {
		perror(NULL);
		exit(-1);
	}
	
	while (readShm(&ptrInfo, &privPackage, &privStr, &privStrMaxLen) && privPackage.filepathLen != 0) {
		printf("\"%s\", %u, %u, %s, %f, %u \n", privStr, privPackage.cantidadClausulas, privPackage.cantidadVariables,
			satResultToString(privPackage.status), privPackage.timeSeconds, privPackage.workerId);
	}

#if DEBUG_PRINTS == 1
	fprintf(stderr, "[View] Closing.\n");
#endif

	resourceClose(&ptrInfo);
	return 0;
}