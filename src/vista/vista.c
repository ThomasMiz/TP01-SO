#include <stdio.h>
#include "./../shared/shmHandler.h"

int main(int argc, const char* argv[]) {
	printf("Yo soy el proceso vista!\n");
	printf("Shared string: %s\n", getSomeSharedString());

	/*
	ResourcesPtr resources = resourcesOpen(SHM SIZE, SHM NAME, SEMP MTX NAME, SEMP ALL NAME);

	GUARDAR LOS PATH

	LOOP PRINCIPAL (FINISH?) {
		WAIT -> BOTH SEMP
		INFO = READ FROM SHM
		POST
		PRINTF
		FILES -1
	}

	CLOSE RESOURCES

	*/

	return 0;
}