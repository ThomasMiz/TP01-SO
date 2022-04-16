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

	char* shmName = (char* ) malloc((10) * sizeof(char));;
	size_t shmSize;
	TPackage* privateBuff = (TPackage*) malloc(sizeof(TPackage));
	
	if (argc == 1) {
		fscanf(stdin, "%10s:%lu", shmName, &shmSize);
	}
	if (argc == 2) {
		scanf(argv[1], "%10s:%lu", shmName, &shmSize);
		// int iLen = strlen(argv[1]);
		// char* sInput = (char* ) malloc((iLen+1) * sizeof(char));
		// strcpy(sInput, argv[1]);
		// printf("Recieved argument => %s \n", argv[1]);
		// char* sSeparator = ":";
		// char* pToken = strtok(sInput, sSeparator);
		// if(pToken == NULL) {
			// fprintf(stderr, "Wrong input, shmName arg NULL \n");
			// exit(EXIT_FAILURE);
		// }
		// shmName = pToken;
		// pToken = strtok(NULL, sSeparator);
		// if(pToken == NULL) {
			// fprintf(stderr, "Wrong input, shmSize arg NULL \n");
			// exit(EXIT_FAILURE);
		// }
		// shmSize = atoi(pToken);
		// if (shmSize > MAX_SHM_SIZE) {
			// fprintf(stderr, "Shared memory space requested too big \n");
			// exit(EXIT_FAILURE);
		// }
	}
	else if (argc > 2) {
		fprintf(stderr, "Too many arguments: %d \n", argc);
        exit(EXIT_FAILURE);
	}
	else {
		fprintf(stderr, "Arguments expected \n");
		exit(EXIT_FAILURE);
	}
	
	TSharedMem ptrInfo;
	resourceOpen(shmName, shmSize, &ptrInfo);
	TSharedMemContext* sharedMemContext = ptrInfo.shmStart;
	
	sem_post(&sharedMemContext->semCanWrite);
	
	while (1) {
		
		readShm(&ptrInfo, privateBuff);
		printf("\"%s\", %u, %u, %s, %f, %u \n", privateBuff->filepath, privateBuff->result->cantidadClausulas,
			privateBuff->result->cantidadVariables, privateBuff->result->status,
			privateBuff->result->timeSeconds, privateBuff->workerId);
	}
	
	resourceClose(&ptrInfo);
	free(shmName);
	free(privateBuff);

	return 0;
}