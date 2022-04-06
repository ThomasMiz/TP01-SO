#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "app.h"
#include "worker.h"
#include "./../shared/shared.h"

/** Spawns the specified amount of workers, creates the pipes to
 * send tasks to them and places the write end fd of those pipes
 * in the specified array. */
void spawnWorkers(unsigned int workerCount, int pipeWriteFds[], int* resultPipeReadFd);

int main(int argc, const char* argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "Error: no input files.\n");
		return EXIT_CODE_NOT_ENOUGH_PARAMS;
	}
	
	// The amount of workers to spawn will be the amount of files
	// we have to process, clamped to the maximum allowed.
	unsigned int workerCount = argc - 1;
	if (workerCount > MAX_WORKERS)
		workerCount = MAX_WORKERS;
	
	int resultPipeFd;
	int requestPipeWriteFds[workerCount];
	spawnWorkers(workerCount, requestPipeWriteFds, &resultPipeFd);
	
	fprintf(stderr, "App process sending some shit to workers\n");
	
	// Pipe test: send a simple string to all workers.
	for (int i=0; i<workerCount; i++) {
		write(requestPipeWriteFds[i], "hola! :D", 8);
		close(requestPipeWriteFds[i]);
	}
	
	// Pipe test: send to STDOUT what comes in through results.
	while (1) {
		char c;
		int n;
		if ((n = read(resultPipeFd, &c, 1)) == -1) {
			fprintf(stderr, "App process found error while reading: ");
			perror(NULL);
			break;
		}
		
		if (n == 0)
			break;
		
		write(STDOUT_FILENO, &c, n);
	}
	
	printf("\nApp process closing.\n");
	return 0;
}

void spawnWorkers(unsigned int workerCount, int pipeWriteFds[], int* resultPipeReadFd) {
	int tmpFd[2];
	
	// We create the pipe through which results are sent back.
	if (pipe(tmpFd)) {
		fprintf(stderr, "Error: Failed to create result pipe: ");
		perror(NULL);
		printf("Aborting.\n");
		exit(EXIT_CODE_CREATE_PIPE_FAILED);
		return;
	}
	
	*resultPipeReadFd = tmpFd[0];
	int resultPipeWriteFd = tmpFd[1];
	
	for (unsigned int i=0; i<workerCount; i++) {
		// We attempt to create a pipe and handle any errors.
		if (pipe(tmpFd)) {
			fprintf(stderr, "Error: Failed to create pipe for worker %i: ", i);
			perror(NULL);
			printf("Aborting.\n");
			exit(EXIT_CODE_CREATE_PIPE_FAILED);
			return;
		}
		
		pipeWriteFds[i] = tmpFd[1];
		
		// We create the child process and handle any errors.
		pid_t forkResult = fork();
		if (forkResult == -1) {
			fprintf(stderr, "Error: Failed to create fork for worker %i: ", i);
			perror(NULL);
			printf("Aborting.\n");
			exit(EXIT_CODE_FORK_FAILED);
			return;
		}
		
		// The child process redirects fds and calls workerMain.
		if (forkResult == 0) {
			// Close the write end of the request pipe.
			close(tmpFd[1]);
			// Close the read end of the result pipe.
			close(*resultPipeReadFd);
			// Redirect STDOUT to the write end of the result pipe.
			dup2(resultPipeWriteFd, STDOUT_FILENO);
			// Redirect STDIN to the read end of the request pipe.
			dup2(tmpFd[0], STDIN_FILENO);
			workerMain(i);
			exit(-1); // Just in case
			return;
		}
		
		// The parent process closes the read end of the request
		// pipe and continues.
		close(tmpFd[0]);
	}
	
	// Close the write end of the result pipe. The parent process
	// doesn't need to write to it.
	close(resultPipeWriteFd);
}