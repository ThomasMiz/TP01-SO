// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "shmViewHandler.h"
#include "./../shared/shmHandler.h"
#include "./../shared/constants.h"

int main(int argc, const char* argv[]) {		//Recibe nombre:size

	if (argc > 2) {
		printf("[View] Error: Too many arguments: %d \n", argc);
        return EXIT_FAILURE;
	}
	
	// Disable buffering the output.
	setvbuf(stdout, NULL, _IONBF, 0);

	char shmName[11];
	unsigned long shmSize = 0;
	
	int matches;
	if (argc == 2)
		matches = sscanf(argv[1], "%10[/a-zA-Z]:%10lu", shmName, &shmSize);
	else
		matches = fscanf(stdin, "%10[/a-zA-Z]:%10lu", shmName, &shmSize);
	
	if (matches < 2) {
		printf("[View] Error: shared memory signature not in the right format.\n");
		return EXIT_FAILURE;
	}
	
	if (shmSize > MAX_SHM_SIZE) {
		printf("[View] Error: invalid shared memory size: %lu but max allowed is %lu.\n", shmSize, (unsigned long)MAX_SHM_SIZE);
		return EXIT_FAILURE;
	}
	if (shmSize <= (sizeof(TPackage) + sizeof(TSharedMemContext))) {
		printf("[View] Error: invalid shared memory size: need at least %lu bytes.\n", (unsigned long) (sizeof(TPackage) + sizeof(TSharedMemContext)));
		return EXIT_FAILURE;
	}
	
#if DEBUG_PRINTS == 1
	printf("[View] Connecting to %s with size %lu.\n", shmName, shmSize);
#endif

	TSharedMem ptrInfo;
	if(!resourceOpen(shmName, shmSize, &ptrInfo)) {
		return EXIT_FAILURE;
	}
	TSharedMemContext* sharedMemContext = ptrInfo.shmStart;
	
	TPackage privPackage;
	char* privStr = NULL;
	size_t privStrMaxLen = 0;
	
#if DEBUG_PRINTS == 1
	fprintf(stderr, "[View] Ack by posting to semCanRead.\n");
#endif

	if (sem_post(&sharedMemContext->semCanWrite)) {
		perror("[View] Failed to send ack to app process.\n");
		resourceClose(&ptrInfo);
		return 0;
	}
	
	// The output is printed as a table.
	// Print the first line, which indicates what each column represents.
	printf("%-50s | %-10s | %-10s | %-10s | %-10s | %-3s\n", "Archivo", "Clausulas", "Variables",
			"Resultado", "Tiempo", "Worker ID");
	
	while (readShm(&ptrInfo, &privPackage, &privStr, &privStrMaxLen) && privPackage.filepathLen != 0) {
		if (privPackage.status == Error) {
			printf("%-50s | %-10s | %-10s | %-10s | %-10f | %-3u\n", privStr, "", "",
				satResultToString(privPackage.status), privPackage.timeSeconds, privPackage.workerId);
		} else {
			printf("%-50s | %-10u | %-10u | %-10s | %-10f | %-3u\n", privStr, privPackage.cantidadClausulas, privPackage.cantidadVariables,
				satResultToString(privPackage.status), privPackage.timeSeconds, privPackage.workerId);
		}
	}

#if DEBUG_PRINTS == 1
	fprintf(stderr, "[View] Closing.\n");
#endif

	resourceClose(&ptrInfo);
	free(privStr);
	return 0;
}