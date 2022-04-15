#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "shmViewHandler.h"
#include "./../shared/constants.h"
#include "./../shared/shmHandler.h"

int main(int argc, const char* argv[]) {		//Recibe nombre:size

	char* shmName;
	size_t shmSize;
	
	if (argc == 2) {
		int iLen = strlen(argv[1]);
		char* sInput = (char* ) malloc((iLen+1) * sizeof(char));
		strcpy(sInput, argv[1]);
		printf("Recieved argument => %s \n", argv[1]);
		char* sSeparator = ":";
		char* pToken = strtok(sInput, sSeparator);
		if(pToken == NULL) {
			fprintf(stderr, "Wrong input, shmName arg NULL \n");
			exit(EXIT_FAILURE);
		}
		shmName = pToken;
		pToken = strtok(NULL, sSeparator);
		if(pToken == NULL) {
			fprintf(stderr, "Wrong input, shmSize arg NULL \n");
			exit(EXIT_FAILURE);
		}
		shmSize = atoi(pToken);
		if (shmSize > MAX_SHM_SIZE) {
			fprintf(stderr, "Shared memory space requested too big \n");
			exit(EXIT_FAILURE);
		}
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
	
	while (1) {
		
		sem_wait(&sharedMemContext->semCanRead);
		if (sharedMemContext->bytesSent == 0) {
			fprintf(stderr, "[ERR] bytesSent=0\n");
			exit(EXIT_FAILURE);
		}
		else {
			//copiar buffer a local
		}
		sem_post(&sharedMemContext->semCanWrite);
		//printeo data y si es comando de terminado cerrar salir del while
	}
	
	resourceClose(&ptrInfo);

	return 0;
}