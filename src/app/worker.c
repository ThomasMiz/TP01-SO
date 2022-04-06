#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "worker.h"

void workerMain(unsigned int workerId) {
	// Pipe test: just uppercase what comes through STDIN to STDOUT.
	while (1) {
		char c;
		int n = read(STDIN_FILENO, &c, 1);
		if (n == -1) {
			fprintf(stderr, "Worker id %u error while reading: ", workerId);
			perror(NULL);
			break;
		}
		
		// Read returns 0 when the end of the file was reached.
		if (n == 0)
			break;
		
		if (c >= 'a' && c <= 'z')
			c = c - ('a' - 'A');
		write(STDOUT_FILENO, &c, n);
	}
	
	exit(0);
}