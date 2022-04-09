#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "worker.h"

int main(int argc, const char* argv[]) {
	
	if (argc <= 1) {
		fprintf(stderr, "[Worker] Error: not enough parameters. Workers must receive their id as their only parameter.\n");
		exit(EXIT_CODE_NOT_ENOUGH_PARAMS);
	}
	
	unsigned int workerId = atoi(argv[1]);
	
	// Pipe test: just uppercase what comes through STDIN to STDOUT.
	while (1) {
		char c;
		int n = read(STDIN_FILENO, &c, 1);
		if (n == -1) {
			fprintf(stderr, "[Worker %u] Error while reading: ", workerId);
			perror(NULL);
			continue;
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